/* 
 * tsh - A tiny shell program with job control
 * 
 * Brigham Andersen - abrigham
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

/* Misc manifest constants */
#define MAXLINE    1024   /* max line size */
#define MAXARGS     128   /* max args on a command line */
#define READ_END      0   /* read end of pipe */
#define WRITE_END     1   /* write end of pipe */
#define CHILD_PROCESS 0   /* child process of fork */
#define TRUE          1   /* boolean substitute */
#define FALSE         0   /* boolean substitute */

/* Global variables */
extern char **environ;      /* defined in libc */
char prompt[] = "tsh> ";    /* command line prompt (DO NOT CHANGE) */
int verbose = 0;            /* if true, print additional output */
char sbuf[MAXLINE];         /* for composing sprintf messages */

/* Function prototypes */

/* Here are the functions that you will implement */
void eval(char *cmdline);
int builtin_cmd(char **argv);

/* Here are helper routines that we've provided for you */
int parseline(const char *cmdline, char **argv); 
int parseargs(char **argv, int *cmds, int *stdin_redir, int *stdout_redir);

void usage(void);
void unix_error(char *msg);
void app_error(char *msg);
typedef void handler_t(int);

/* EXTRA WRAPPER FUNCTIONS FOR SYSTEM CALLS - See bottom of file for function bodies */
pid_t Fork(void);
int Dup2(int fd1, int fd2);
int Pipe(int* pipedes);
void Execve(const char *filename, char *const argv[], char *const envp[]);
pid_t Wait(int *status);
pid_t Waitpid(pid_t pid, int *iptr, int options);
FILE* Fopen(const char *filename, const char *mode);
void Fclose(FILE *fp);
void Setpgid(pid_t pid, pid_t pgid);
void Close(int fd);


/*
 * main - The shell's main routine 
 */
int main(int argc, char **argv) 
{
    char c;
    char cmdline[MAXLINE];
    int emit_prompt = 1; /* emit prompt (default) */

    /* Redirect stderr to stdout (so that driver will get all output
     * on the pipe connected to stdout) */
    dup2(1, 2);

    /* Parse the command line */
    while ((c = getopt(argc, argv, "hvp")) != EOF) {
        switch (c) {
            case 'h':             /* print help message */
                usage();
            break;
            case 'v':             /* emit additional diagnostic info */
                verbose = 1;
            break;
            case 'p':             /* don't print a prompt */
                emit_prompt = 0;  /* handy for automatic testing */
            break;
        default:
            usage();
        }
    }

    /* Execute the shell's read/eval loop */
    while (1) {
        /* Read command line */
        if (emit_prompt) {
            printf("%s", prompt);
            fflush(stdout);
        }
        if ((fgets(cmdline, MAXLINE, stdin) == NULL) && ferror(stdin))
            app_error("fgets error");
        if (feof(stdin)) { /* End of file (ctrl-d) */
            fflush(stdout);
            exit(0);
        }

        /* Evaluate the command line */
        eval(cmdline);
        fflush(stdout);
        fflush(stdout);
    } 

    exit(0); /* control never reaches here */
}
  
/* 
 * eval - Evaluate the command line that the user has just typed in
 * 
 * If the user has requested a built-in command (quit, jobs, bg or fg)
 * then execute it immediately. Otherwise, fork a child process and
 * run the job in the context of the child. If the job is running in
 * the foreground, wait for it to terminate and then return.  Note:
 * each child process must have a unique process group ID so that our
 * background children don't receive SIGINT (SIGTSTP) from the kernel
 * when we type ctrl-c (ctrl-z) at the keyboard.  
*/
void eval(char *cmdline) 
{
    char *argv[MAXARGS];
    pid_t pid;
    int cmds[MAXARGS];
    int stdin_redir[MAXARGS];
    int stdout_redir[MAXARGS];
    int childPids[MAXARGS];
    int pipes[MAXARGS][2];  // Making max number of pipes, can also just juggle 2 pipes like pipes[2].  Make sure to initialize numCmds - 1 pipes
    int groupId;

    parseline(cmdline, argv); 

    // Ignore empty lines
    if (argv[0] == NULL)
        return;

    // If it's a built-in cmd
    if (builtin_cmd(argv) == 1) 
        return;

    // If a custom (not built-in) cmd, run this
    int numCmds = parseargs(argv, cmds, stdin_redir, stdout_redir); 

    // Make pipes (num of pipes is 1 less than numCmds - they go between cmds)
    for (int i = 0; i < numCmds - 1; i++) {
        Pipe(pipes[i]);
    }

    for (int i = 0; i < numCmds; i++) {
        pid = Fork();

        // Child (for each cmd)
        if (pid == CHILD_PROCESS) {
            int redirectedInput = FALSE;
            int redirectedOutput = FALSE;
            
            // If there's an input file, redirect stdin to input file
            if (stdin_redir[i] > 0) {
                redirectedInput = TRUE;
                FILE *inFile = Fopen(argv[stdin_redir[i]], "r");
                Dup2(fileno(inFile), STDIN_FILENO);
                Fclose(inFile);
            }

            // If there's an output file, redirect stdout to output file
            if (stdout_redir[i] > 0) {
                redirectedOutput = TRUE;
                FILE *outFile = Fopen(argv[stdout_redir[i]], "w");
                Dup2(fileno(outFile), STDOUT_FILENO);
                Fclose(outFile);
            }

            // Redirect stdout to write end (run this for all pipes but the last and if the output wasn't redirected to a file)
            if (i < numCmds - 1 && !redirectedOutput) {
                Dup2(pipes[i][WRITE_END], STDOUT_FILENO);
            }

            // Redirect stdin to read end (run this for all pipes but the first and if the input wasn't redirected to a file)
            if (i > 0 && !redirectedInput) {
                Dup2(pipes[i - 1][READ_END], STDIN_FILENO);
            }
        
            // Close all pipes (even the ones you never used)
            for (int j = 0; j < numCmds - 1; j++) {
                Close(pipes[j][READ_END]);
                Close(pipes[j][WRITE_END]);
            }

            // Execute cmd
            Execve(argv[cmds[i]], &argv[cmds[i]], environ);
        }

        // Parent (for each cmd)
        if (pid != CHILD_PROCESS) {
            childPids[i] = pid; // Save child pid

            groupId = childPids[0]; // Set group id to first child pid
            Setpgid(childPids[i], groupId);
        }    
    }

    // Parent closes pipes
    for (int i = 0; i < numCmds - 1; i++) {
        Close(pipes[i][READ_END]);
        Close(pipes[i][WRITE_END]);
    }

    // Parent waits for kids
    for (int i = 0; i < numCmds; i++) {
        int status;
        Waitpid(childPids[i], &status, 0);
    }
    
    return;
}

/* 
 * parseargs - Parse the arguments to identify pipelined commands
 * 
 * Walk through each of the arguments to find each pipelined command.  If the
 * argument was | (pipe), then the next argument starts the new command on the
 * pipeline.  If the argument was < or >, then the next argument is the file
 * from/to which stdin or stdout should be redirected, respectively.  After it
 * runs, the arrays for cmds, stdin_redir, and stdout_redir all have the same
 * number of items---which is the number of commands in the pipeline.  The cmds
 * array is populated with the indexes of argv corresponding to the start of
 * each command sequence in the pipeline.  For each slot in cmds, there is a
 * corresponding slot in stdin_redir and stdout_redir.  If the slot has a -1,
 * then there is no redirection; if it is >= 0, then the value corresponds to
 * the index in argv that holds the filename associated with the redirection.
 * 
 */
int parseargs(char **argv, int *cmds, int *stdin_redir, int *stdout_redir) 
{
    int argindex = 0;    /* the index of the current argument in the current cmd */
    int cmdindex = 0;    /* the index of the current cmd */

    if (!argv[argindex]) {
        return 0;
    }

    cmds[cmdindex] = argindex;
    stdin_redir[cmdindex] = -1;
    stdout_redir[cmdindex] = -1;
    argindex++;
    while (argv[argindex]) {
        if (strcmp(argv[argindex], "<") == 0) {
            argv[argindex] = NULL;
            argindex++;
            if (!argv[argindex]) { /* if we have reached the end, then break */
                break;
	        }
            stdin_redir[cmdindex] = argindex;
	    } else if (strcmp(argv[argindex], ">") == 0) {
            argv[argindex] = NULL;
            argindex++;
            if (!argv[argindex]) { /* if we have reached the end, then break */
                break;
	        }
            stdout_redir[cmdindex] = argindex;
	    } else if (strcmp(argv[argindex], "|") == 0) {
            argv[argindex] = NULL;
            argindex++;
            if (!argv[argindex]) { /* if we have reached the end, then break */
                break;
	        }
            cmdindex++;
            cmds[cmdindex] = argindex;
            stdin_redir[cmdindex] = -1;
            stdout_redir[cmdindex] = -1;
	    }
        argindex++;
    }

    return cmdindex + 1;
}

/* 
 * parseline - Parse the command line and build the argv array.
 * 
 * Characters enclosed in single quotes are treated as a single
 * argument.  Return true if the user has requested a BG job, false if
 * the user has requested a FG job.  
 */
int parseline(const char *cmdline, char **argv) 
{
    static char array[MAXLINE]; /* holds local copy of command line */
    char *buf = array;          /* ptr that traverses command line */
    char *delim;                /* points to first space delimiter */
    int argc;                   /* number of args */
    int bg;                     /* background job? */

    strcpy(buf, cmdline);
    buf[strlen(buf)-1] = ' ';  /* replace trailing '\n' with space */
    while (*buf && (*buf == ' ')) /* ignore leading spaces */
	    buf++;

    /* Build the argv list */
    argc = 0;
    if (*buf == '\'') {
	buf++;
	delim = strchr(buf, '\'');
    }
    else {
	delim = strchr(buf, ' ');
    }

    while (delim) {
        argv[argc++] = buf;
        *delim = '\0';
        buf = delim + 1;
        while (*buf && (*buf == ' ')) /* ignore spaces */
            buf++;

        if (*buf == '\'') {
            buf++;
            delim = strchr(buf, '\'');
        }
        else {
            delim = strchr(buf, ' ');
        }
    }
    argv[argc] = NULL;
    
    if (argc == 0)  /* ignore blank line */
	return 1;

    /* should the job run in the background? */
    if ((bg = (*argv[argc-1] == '&')) != 0) {
	argv[--argc] = NULL;
    }
    return bg;
}

/* 
 * builtin_cmd - If the user has typed a built-in command then execute
 *    it immediately.  
 *     Returns 1 for a built-in cmd and 0 if not a built-in cmd
 */
int builtin_cmd(char **argv) 
{
    if (strcmp(argv[0], "quit") == 0)
        exit(EXIT_SUCCESS);
    if (strcmp(argv[0], "fg") == 0)
        return TRUE;
    if (strcmp(argv[0], "bg") == 0)
        return TRUE;
    if (strcmp(argv[0], "jobs") == 0)
        return TRUE;
    return FALSE;     /* not a builtin command */
}

/***********************
 * Other helper routines
 ***********************/

/*
 * usage - print a help message
 */
void usage(void) 
{
    printf("Usage: shell [-hvp]\n");
    printf("   -h   print this message\n");
    printf("   -v   print additional diagnostic information\n");
    printf("   -p   do not emit a command prompt\n");
    exit(1);
}

/*
 * unix_error - unix-style error routine
 */
void unix_error(char *msg)
{
    fprintf(stdout, "%s: %s\n", msg, strerror(errno));
    exit(1);
}

/*
 * app_error - application-style error routine
 */
void app_error(char *msg)
{
    fprintf(stdout, "%s\n", msg);
    exit(1);
}

/*
 * EXTRA WRAPPER FUNCTIONS FOR SYSTEM CALLS - Taken from book - http://csapp.cs.cmu.edu/3e/ics3/code/src/csapp.c
 */
pid_t Fork(void) 
{
    pid_t pid;

    if ((pid = fork()) < 0)
	unix_error("Fork error");
    return pid;
}

int Dup2(int fd1, int fd2) 
{
    int rc;

    if ((rc = dup2(fd1, fd2)) < 0)
	unix_error("Dup2 error");
    return rc;
}

int Pipe(int* pipedes) {
    int rc;

    if ((rc = pipe(pipedes) < 0)) {
        unix_error("Pipe error");
    }
    return rc;
}

void Execve(const char *filename, char *const argv[], char *const envp[]) 
{
    if (execve(filename, argv, envp) < 0)
	unix_error("Execve error");
}

pid_t Wait(int *status) 
{
    pid_t pid;

    if ((pid  = wait(status)) < 0)
	unix_error("Wait error");
    return pid;
}

pid_t Waitpid(pid_t pid, int *iptr, int options) 
{
    pid_t retpid;

    if ((retpid  = waitpid(pid, iptr, options)) < 0) 
	unix_error("Waitpid error");
    return(retpid);
}

FILE* Fopen(const char *filename, const char *mode) 
{
    FILE *fp;

    if ((fp = fopen(filename, mode)) == NULL) {
	    unix_error("Fopen error"); 
    }

    return fp;
}

void Fclose(FILE *fp) 
{
    if (fclose(fp) != 0)
	unix_error("Fclose error");
}

void Setpgid(pid_t pid, pid_t pgid) {
    int rc;

    if ((rc = setpgid(pid, pgid)) < 0)
	unix_error("Setpgid error");
    return;
}

void Close(int fd) 
{
    int rc;

    if ((rc = close(fd)) < 0)
	unix_error("Close error");
}