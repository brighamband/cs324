#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
	char *scenario = argv[1];
	int pid = atoi(argv[2]);

	switch (scenario[0]) {
	case '1':
		break;	// No output
	case '2':
		kill(pid, SIGHUP);	// Outputs 1 and 2
		sleep(5);		// Gives time for both 1 and 2 to output before exiting
		break;
	case '3':
		kill(pid, SIGHUP);	// Outputs 1 and 2
		sleep(5);		// Lets them finish outputting
		kill(pid, SIGHUP);	// Outputs 1 and 2 again (can reuse the same signal since the first one has finished processing)
		sleep(5);		// Lets them finish outputting
		break;
	case '4':
		kill(pid, SIGHUP);	// Starts outputting 1 and 2
		sleep(1);		// Gives time for target to receive signal
		kill(pid, SIGINT);	// Starts outputting 1 and 2
		sleep(5);		// Before exiting, the 2 from the first signal prints as well as the 2 from the last signal
		break;
	case '5':
		kill(pid, SIGHUP);	// Outputs 1 and 2
		sleep(5);		// Gives time for them to completely finish
		kill(pid, SIGUSR1);	// Prints a 7 (forks, then prints the child exit code)
		sleep(1);		// Gives time for target to receive signal
		kill(pid, SIGSTKFLT);	// Prints a 10 (tries to wait, but WNOHANG returns immediately, so returns 10 error code for no child processes)
		sleep(1);		// Gives time for target to receive signal
		break;
	case '6':
		kill(pid, SIGHUP);	// Outputs 1 and 2
		sleep(5);		// Gives time for them to completely finish
		kill(pid, SIGSYS);	// Blocks the 7 child output
		sleep(1);		// Gives time for target to receive signal
		kill(pid, SIGUSR1);	// Makes foo > 0
		sleep(1);		// Gives time for target to receive signal
		kill(pid, SIGPWR);	// Sets foo = 6 (since it's > 0)
		sleep(1);		// Gives time for target to receive signal
		kill(pid, SIGTERM);	// Prints foo (6)
		sleep(1);		// Gives time for target to receive signal
		break;
	case '7':
		kill(pid, SIGSYS);	// Blocks the SIGINT signal that would've outputted 1 and 2
		sleep(1);		// Gives time for target to receive signal
		kill(pid, SIGQUIT);	// Outputs 8 and 9 (call to SIGINT doesn't go through immediately BC of block)
		sleep(5);		// Gives time to output 8 and 9 since they sleep
		kill(pid, SIGSYS);		// Unblock the SIGINT signal (now 1 and 2 will output)
		sleep(5);		// Gives time to output 1 and 2 since they sleep
		break;

	}
}

/*
Scenario 1
(No output)

Scenario 2
1
2

Scenario 3
1
2
1
2

Scenario 4
1
1
2
2

Scenario 5
1
2
7
10

Scenario 6
1
2
6

Scenario 7
8
9
1
2
*/