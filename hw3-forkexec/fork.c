#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
	int pid;
	int pipefd[2];

	printf("Starting program; process has pid %d\n", getpid());

	FILE * forkOut = fopen("fork-output.txt", "w");
	fprintf(forkOut, "BEFORE FORK\n");
	fflush(forkOut);

	pipe(pipefd);

	if ((pid = fork()) < 0) {
		fprintf(stderr, "Could not fork()");
		exit(1);
	}

	/* BEGIN SECTION A */
	fprintf(forkOut, "SECTION A\n");
	fflush(forkOut);

	printf("Section A;  pid %d\n", getpid());

	/* END SECTION A */
	if (pid == 0) {
		/* BEGIN SECTION B */
		fprintf(forkOut, "SECTION B\n");
		fflush(forkOut);

		close(pipefd[0]);	// Close read end
		FILE * pipeOut = fdopen(pipefd[1], "w");	// Make file stream for write end
		fputs("hello from Section B\n", pipeOut);	// Write msg to write end

		printf("Section B\n");
		printf("Section B done sleeping\n");

		char *newenviron[] = { NULL };

		printf("Program \"%s\" has pid %d. Sleeping.\n", argv[0], getpid());

		if (argc <= 1) {
			printf("No program to exec.  Exiting...\n");
			exit(0);
		}

		dup2(fileno(forkOut), 1);	// Redir child stdin to go to forkOut stream

		printf("Running exec of \"%s\"\n", argv[1]);
		execve(argv[1], &argv[1], newenviron);
		printf("End of program \"%s\".\n", argv[0]);

		exit(0);

		/* END SECTION B */
	} else {
		/* BEGIN SECTION C */
		fprintf(forkOut, "SECTION C\n");
		fflush(forkOut);

		close(pipefd[1]);	// Close write end
		FILE * pipeIn = fdopen(pipefd[0], "r");	// Make file stream for read end
		char buf[100];	// Read in a line, print to stdout
    fgets(buf, 100, pipeIn);
    printf("%s", buf);

		printf("Section C\n");
		wait(NULL);
		printf("Section C done sleeping\n");

		exit(0);

		/* END SECTION C */
	}
	/* BEGIN SECTION D */
	fprintf(forkOut, "SECTION D\n");
	fflush(forkOut);

	printf("Section D\n");
	/* END SECTION D */
}
