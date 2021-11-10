In this assignment, you will learn and practice basic coding, compilation, environment, and string searching in C, including basic Input/Output.  Read the entire assignment before beginning!

**Usage**

Your program will be run from the command line in the following way:

./catmatch filename

The "./" at the beginning simply means that you are directing the shell to look for the program, catmatch, in the current directory (e.g., ".").  "filename" indicates that the program will take filename to read as a command-line argument.  You can also pass an environment variable CATMATCH_PATTERN, which is a pattern.

Your program will:

1.  Print the process ID to standard error (stderr)--not standard output (stdout)--followed by two newlines.
2.  Open the file specified on the command line
3.  For each line in the file:

1.  read the entire line into a buffer (i.e., "C string"; you will want to declare this as a char array);
2.  if CATMATCH_PATTERN exists as an environment variable, check the line for the pattern specified in the environment;
3.  print the line to standard output (stdout), prefaced with 1 or 0 (and a space), indicating that the pattern was found or not found, respectively; if CATMATCH_PATTERN was not specified, then just print 0 before the line.

Save your file as catmatch.c, and use the following command to compile your program:

gcc -o catmatch catmatch.c

Your code should compile with no warnings!

The "-o" option names the resulting binary being named "catmatch" (as opposed to the default, "a.out").

**Testing**

Consider the file  lorem-ipsum.txt  [Download](https://learningsuite.byu.edu/plugins/Upload/fileDownload.php?fileId=0218e533-tVUw-p4zh-CieU-YC4ff20f6b6e) .  If run with:

./catmatch lorem-ipsum.txt

and the value of the environment var CATMATCH_PATTERN was al, then the output would be:

1234

0 Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor\
1 incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis\
1 nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat.\
0 Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu\
0 fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in\
0 culpa qui officia deserunt mollit anim id est laborum.

(where 1234 is the process ID)

Try with several different patterns.  You can check your work with the command-line program grep. 

**Helps**

-   You will find the following functions helpful:

-   getpid()
-   printf() and fprintf()
-   fopen()
-   fgets()
-   getenv()
-   strstr()

-   Read and use the man page for each of the functions referenced above.  It will make your life easier.
-   See also the man pages for stdout and stderr.
-   If you are unfamiliar with man pages, please read the man page for man itself!

**Remote Compilation and Execution**

1\. Copy the source for your program and the text file to one of the CS lab machines using scp (replace "username" with your CS username):

scp catmatch.c lorem-ipsum.txt username@schizo.cs.byu.edu:

2\. Log in to one of the CS machines using the following command:

$ ssh username@schizo.cs.byu.edu

3\. Run the following command:

$ tmux

Your screen will look similar to how it did before, but note that the shell instance corresponding to the prompt you are seeing is running within tmux, a terminal multiplexer.  The idea is that you can now instantiate other shells on the same remote machine, in different windows displayed alongside one another, disconnect and re-connect to your tmux instance on the remote machine, and more.

4\. Type ctrl-b followed by " (double quotation mark).  This will split the window in tmux horizontally and create a separate shell instance in the lower one.

5\. Run "echo lower left" in the newly created window/shell.

6\. Type ctrl-b followed by % (percent sign).  This will split the lower window in tmux vertically and create a separate shell instance in the lower one.

7\. Run "echo lower left" in the newly created window/shell.

8\. Type ctrl-b followed by the up arrow/cursor key.  This will move control to the upper window--i.e., the first window that was created.

9\. Run "echo upper" in the upper window/shell.

10\. In the upper window/shell, compile and run your catmatch program, the same way as you did in your local development environment.

11\. Type ctrl-b followed by d (the letter d) to detach from your current tmux instance.

Note that at this point, you could log out of the machine you are currently working on (on which tmux is running), or you could be disconnected inadvertently, and your tmux instance would still be running, detached.  However, you need to know which machine it was running on (i.e., because schizo simply redirects you to *some* machine in the CS labs).

12\. Run the following command:

$ "tmux attach"

This should reattach you to the tmux instance that you were working on earlier, and it should look exactly as it did when you detached.

13\. Type ctrl-d in each of the windows in your tmux instance, to close each shell and (when the last one closes) the tmux instance itself.  ctrl-d essentially passes an end-of-file, so the shell knows that its input has finished--its signal to terminate!

**Submission**

Modify your catmatch.c to include a comment in the code that has the following text: "I COMPLETED THE TMUX EXERCISE!"  Make sure your program still compiles properly!  Then upload catmatch.c to the assignment page on LearningSuite.