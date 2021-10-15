The purpose of this assignment is to give you hands-on experience with signals.  Code is provided that has handlers installed for various signals.  You will interact with the existing code and change its behavior using the kill function.

**Preparation**

a. Download signals.tar.gz and un-tar/gzip it to a directory:

signals.tar.gz  [Download](https://learningsuite.byu.edu/plugins/Upload/fileDownload.php?fileId=d24fab2e-Yw6O-H6dg-f7WY-sGb1023e3d9f)

$ tar -zxvf signals.tar.gz

(Note: LearningSuite does something weird to my tar.gz files, so you might need to do this:

$ gunzip signals.tar.gz; tar -xvf signals.tar

)

In the newly created signals directory, you will find three files: signals.c, killer.c, and Makefile, each described briefly hereafter:

-   **signals.c**

1.  installs signal handlers for various signals
2.  calls fork()
3.  child spins in a mostly uneventful sleep loop for 30 seconds.  In addition to sleeping, it checks each iteration the value of the variable block; if true (non-zero), it uses sigprocmask to block SIGINT and SIGCHLD.
4.  the parent calls execve() on another executable passed in on the command line (killer), which then becomes the code for the parent process.

-   **killer.c**

1.  Takes scenario (integer, 1 - 7) and (child) process ID from the command line.
2.  Depending on the scenario, sends signals to the child process using the kill() function to get the desired output for each scenario.

-   **Makefile**

1.  Compiles both executables by running "make".
2.  Runs all scenarios by running "make test".

To run just one specific scenario, use the following:

./signals ./killer 1

(substitute 1 with whatever scenario you want to run, 1 through 7)

**Desired Output:**

Here is the desired output for each scenario:

**Scenario 1**\
(No output)

**Scenario 2**\
1\
2

**Scenario 3**\
1\
2\
1\
2

**Scenario 4**\
1\
1\
2\
2

**Scenario 5**\
1\
2\
7\
10

**Scenario 6**\
1\
2\
6

**Scenario 7**\
8\
9\
1\
2

**Code**

The only code you will modify is the case statements in in the switch statement in killer.c.  And you will *only* use the following functions: sleep() and kill().  The first argument to pid will always be pid (i.e., the process ID corresponding to the child process).  The second will be one of the signals for which a handler is defined.  The sleep() function is just used to help avoid race conditions, so you can reliably plan on a statement in the code where the signal will be received.

For example, the code for a scenario might look like this:

case '1':\
    kill(pid, SIGINT);\
    sleep(1);\
    kill(pid, SIGTERM);\
    sleep(3);\
    kill(pid, 31);\
    break;

(Please note that the statements above are just to show you an example of how to apply kill() and sleep() calls to the killer.c file; they do not necessarily do anything useful.)

The trick, of course, is what signals to send and at what times, to get the desired output.  Look at the handlers closely to see what they do, and practice what you know about signal behavior to send the right signals at the right times.

The one special case is sig_handler7().  It appears as though it is just changing a global variable.  But changing that global variable in the handler results in SIGINT and SIGCHLD being blocked--or unblocked--in the main loop.  Those blocks would have been put right in handler, which might have been more intuitive, but unfortunately the set of blocked signals is overwritten (restored, actually) when a handler returns, and that won't be helpful for the exercise at hand.  You are welcome to still think of them as having been carried out right in the handler!

You are not allowed to send signals other than those for which handlers are installed in signals.c.  In particular, you cannot use SIGKILL.

**Hints**

-   ctrl-c will not work for terminating your processes during development because SIGINT has been overridden with a handler.  You can, however, use ctrl-z, then "kill -9 %1".  This will send the SIGKILL signal, which cannot be overridden.
-   You can modify signals.c all you want with comments, debug statements and whatever you want, if it will help you.  In the end, you will just be uploading your killer.c, and we will use a stock signals.c to test against.
-   You will likely need a sleep() statement after every kill() call.  At minimum, you should sleep for 1 second, but depending on what you are trying to accomplish, you might need to sleep for longer.  This is to give the target process enough time to receive the signal.

**Submission:**

Please upload your killer.c file to the assignment page on LearningSuite.

**Grading**

-   14 points - 2 for each correct scenario output