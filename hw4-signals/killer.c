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
		sleep(1);		// Gives time for 1 to output from previous signal
		kill(pid, SIGINT);	// Starts outputting 1 and 2
		sleep(5);		// Before exiting, the 2 from the first signal prints as well as the 2 from the last signal
		break;
	case '5':
		kill(pid, SIGHUP);	// Outputs 1 and 2
		sleep(5);		// Gives time for them to completely finish
		kill(pid, SIGUSR1);	// Prints a 7 (forks, then prints the child exit code)
		sleep(1);		// Lets waiting for child/forking finish first before continuing
		kill(pid, SIGSTKFLT);	// Prints a 10 (tries to wait, but WNOHANG returns immediately, so returns 10 error code for no child processes)
		break;
	case '6':
		kill(pid, SIGHUP);	// Outputs 1 and 2
		sleep(6);
		kill(pid, SIGSYS);	// Silences the 7 child output
		kill(pid, SIGUSR1);	// Makes foo > 0
		sleep(2);
		kill(pid, SIGPWR);	// Sets foo = 6 (since it's > 0)
		kill(pid, SIGTERM);	// Prints foo (6)
		break;
	case '7':
		kill(pid, SIGQUIT);
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