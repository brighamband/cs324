Setup

1\. Download concurrency.tar.gz and un-tar/gzip it to a directory:

concurrency.tar.gz  Download

$ tar -zxvf concurrency.tar.gz

(Note: LearningSuite does something weird to my tar.gz files, so you might need to do this:

$ gunzip concurrency.tar.gz; tar -xvf concurrency.tar

)

2\. Run "make" to build four servers: echoserveri, echoserverp, echoservert, and echoservert_pre.  These are versions of the echo server that use no concurrency, process-based (using fork()) concurrency, simple thread-based concurrency (threads created on the fly), and thread-based concurrency using a thread-pool.

3\. Start a tmux session with five panes open.  You are welcome to arrange them however you want, but it might be easiest to do it something like this:

----------------

| 1  | 2  | 3  |

----------------

|      4       |

----------------

|      5       |

----------------

Part 1: no concurrency

Start the "echoserveri" server in a pane (e.g., pane 4) using the following command line:

./echoserveri 5599

(The port is arbitrary - it can be anything above 1023).

In three other panes (e.g., 1 - 3), run the following:

$ nc localhost 5599

(the port should be consistent with what you used to start the server)

After all three are running, type some text in the first of the three panes running "nc", and press enter.  Repeat with the second and third panes.

In a fifth pane (e.g., pane 5), run the following:

$ ps -Lo user,pid,ppid,nlwp,lwp,state,ucmd -C echoserveri | grep ^username\\\|USER

(Note: use your own username in place of "username", and use the actual name of the binary in place of "echoserver")

The "ps" command lists information about processes that currently exist on the system.  The -L option tells us to show threads (lightweight processes) as if they were processes.  The "-o" option is used to show only the following fields:

user: the username of the user running the process (should be your CS username)

pid: the process ID of the process

ppid: the process ID of the parent process

nlwp: the number of threads (light-weight processes) being run

lwp: the thread ID of the thread

state: the state of the process, e.g., Running, Sleep, Zombie

ucmd: the command executed.

While some past homework assignments required you to use the ps command without a pipeline (e.g., to send its output grep), ps has the shortcoming that it can't simultaneously filter by both command name and user, so the above command line is a workaround.

1\. Show the output from the ps command.

2\. From the ps output, how many processes and how many threads are running, and why?  Use the PID and LWP to identify different threads or processes.

3\. Enter "Ctrl-c" on the window in which "nc" was first executed to interrupt it.  What happens to the "nc" processes in the other windows?

Stop the server by using "Ctrl-c" in the appropriate pane.

Part 2: Process-based Concurrency

Repeat the exercises from Part 1 (except question 3), replacing all instances of "echoserveri" with "echoserverp".  Answer questions 1 and 2 for "echoserverp" as questions 4 and 5.

Part 3: Simple Thread-based Concurrency

Repeat the exercises from Part 1 (except question 3), replacing all instances of "echoserveri" with "echoservert".  Answer questions 1 and 2 for "echoservert" as questions 6 and 7.

Part 4: Threadpool-based Concurrency

Repeat the exercises from Part 1 (except question 3), replacing all instances of "echoserveri" with "echoservert_pre".  Answer questions 1 and 2 for "echoservert_pre" as questions 8 and 9.

Part 5: Concurrency Review

Name at least one pro and one con to each of the concurrent programming paradigms in this assignment.  You can use your own observations, as well as the book chapter.

10\. echoserverp

11\. echoservert

12\. echoservert_pre

Consider the code in echoservert_pre.c for the following questions:

13\. Which statement of code represents the producer role?  How many producer threads are there?

14\. Which statement of code represents the consumer role?  How many consumer threads are there?

Consider the code in sbuf.c for the following questions (P() is equivalent to sem_wait() and V() is equivalent to sem_post()):

15\. On line 29, what happens when the value of slots is zero?

16\. In the case that a single consumer thread is sleeping at line 42, waiting for a non-zero value of items, what line of code will be executed by a producer to wake that thread up?

Part 6: HTTP Parser

Download http_parser.c:

http_parser.c  Download

Complete the functions is_complete_request() and parse_request(), such that main() produces the correct output.  You will find the string functions (man "string") very useful for this, including strcpy(), strstr(), strchr(), and more!

Part 7: CGI Program

Write a CGI program in C.  Name your file cgiprog.c.  Your program should do the following:

Finish sending the headers for the HTTP response to the client.  The Web server will have sent the first line of the response and some headers.  Your program should send the headers indicating the type of the content (which will be "text/plain" and the length of the content, which is just the length (in bytes) of the response body.

Sends the following string to the client as a response body: "The query string is: Q" But replace "Q" with the actual query string provided by the client.

Hint: this will be much simpler than you might think--perhaps even simpler than the program you wrote for your first homework assignment.  But you might need to review CGI from the book and slides.  Also, you should recall that there is a way that you can test it without a Web server (Hint: think of HW1 and HW2).

To turn in:

Please submit the following:

The answers to parts 1 - 5 in a single file called hw6.txt.

Your modified http_parser.c

Your cgiprog.c

Use the following command to tar/gzip your files:

tar -zcvf hw6.tar.gz hw6.txt http_parser.c cgiprog.c

Submit hw6.tar.gz to LearningSuite.