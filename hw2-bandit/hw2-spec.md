The purpose of this assignment is to familiarize you with working in a shell environment, including redirection, pipelining, backgrounding, and more.  Read the entire assignment before beginning!

**Preparation**

NOTE: You MUST do this part from BYU's campus network. There are several ways to accomplish this:

-   Bring your laptop to campus, connect to BYU campus network, and run the commands from your laptop.
-   Log on to a CS lab machine, and run the commands from the lab machine.
-   From off campus, log on to a CS lab machine using "ssh username@schizo.cs.byu.edu" (replacing "username" with your actual CS username), and run the commands from the lab machine.
-   From off campus, connect to the [BYU CS Virtual Private Network (VPN)](https://docs.cs.byu.edu/doku.php?id=vpn-configuration-and-use), and run the commands from your own machine.  This will make your machine think it is on BYU's campus.

**Description**

1\. Use the SSH program to log in remotely to the computer imaal.byu.edu with username "bandit0" and password "bandit0":

$ ssh bandit0@imaal.byu.edu

2\. Follow the instructions in the "readme" file to get the password for Level 1.

3\. Close out your session to log out of imaal.byu.edu (ctrl-d or "exit").

4\. Use SSH to again log in to imaal.byu.edu, this time with username bandit1 and the password you obtained for Level 1.

5\. Repeat steps 2 through 4 through Level 9, such that you can log in to imaal.byu.edu successfully as the bandit9 user.

For each level, you need use a combination of input/output redirection and/or pipelining, such that you can get a single pipeline command (i.e., a "one-liner") to output *just* the password for the next level, on a single line.  In some cases, the pipeline might just contain a single command (e.g., for learning the level 1 password).  For most cases, however, more than one command is required.  The exception to this is learning the password for level 8 (i.e., as the bandit7 user).  Please see the next bullet point for that.

When learning the password for level 8 (i.e., as the bandit7 user), the suspend/resume does not need to be done as part of the "one-liner".  Those require keystrokes after the program has been executed.  Just report the single command.

You will be submitting both the commands you used and the output from them, such that it looks something like this (this is contrived):

bandit0@bandit:~$ foo\
0G3wlqW6MYydw4jQJb99pW8+uISjbJhe

bandit1@bandit:~$ grep bar somefile.txt | awk '{ print $8 }' | base64 -d\
xJJHpfRpbE7F2cAt8+V9HLEoZEzZqvi+

...

Note that three commands were used in the example pipeline above: grep, awk, and base64.  The output (stdout) of grep was connected to the input (stdin) of the awk command, and the output (stdout) of awk was connected to the stdin of the base64 command.  There was no further command in the pipeline, so base64's output simply goes to the console.

**Helps**

Here are some commands that you might use to help you:

-   awk
-   base64
-   cat
-   curl
-   cut
-   dig
-   grep
-   gzip
-   head
-   md5sum
-   sha1sum
-   sort
-   tar
-   uniq

A few other helps:

-   Use the man pages to learn about a command, as they are the primary documentation!  You can also find helpful examples on the Web.
-   Where a pipelined command begins with a command that can receive input from stdin, and the initial input is a file, one way of doing it is to use "<" to open the file and send it to the stdin of the first command.
-   To suspend the process currently running in the foreground, use ctrl-z.  Use "fg" to resume.  For more information, See the sections on REDIRECTION, Pipelines (under SHELL GRAMMAR), and JOB CONTROL in the bash man page (man bash).
-   You can redirect stderr output to /dev/null by adding 2> /dev/null to the end of a command.
-   The awk command is pretty extensive and indeed includes a whole language.  However, one of the command uses is to print a single space-delimited field from every line.  For example, a simple awk script to print out just the second field of text from every line, the following command would work: awk '{ print $2 }'
-   dig and curl and are commands used to issue a request to a Domain Name System (DNS) server and HyperText Transfer Protocol (HTTP) server, respectively.  You can try them out with different domain names, types, or URLs, to see how they work, but you shouldn't need to do anything fancy with them for this assignment.  You will find the "+short" option useful for dig.
-   You might feel overwhelmed with the "pipeline" aspect of this assignment.  To help you out, build the pipeline gradually.  For example, in the above example, run just "grep bar somefile.txt" to see what the output is.  Then run "grep bar somefile.txt | awk ' { print $8 }'".  Finally, when that is working, run the whole thing: "grep bar somefile.txt | awk '{ print $8 }' | base64 -d".

**Submission**

Create a file bandit.txt that has the following format:

level1:\
PASSWORD1\
PIPELINE1\
level2:\
PASSWORD2\
PIPELINE2\
...

PASSWORD1 represents the password for level 1 and PIPELINE1 is the actual pipeline of commands (i.e., "one-liner") you used to get that password while logged in as bandit0, etc.  Note that the format above is important, as it will allow your assignment to be graded automatically.

Then upload bandit.txt to the assignment page on LearningSuite.