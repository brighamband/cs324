#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
	char *scenario = argv[1];
	int pid = atoi(argv[2]);

	switch (scenario[0]) {
	case '1':
		break;
	case '2':
		kill(pid, SIGHUP);
		sleep(6);
		break;
	case '3':
		kill(pid, SIGHUP);
		sleep(6);
		kill(pid, SIGHUP);
		sleep(6);
		break;
	case '4':
		kill(pid, SIGHUP);
		sleep(2);
		kill(pid, SIGINT);
		sleep(6);
		break;
	case '5':
		kill(pid, SIGHUP);
		sleep(6);
		kill(pid, SIGUSR1);
		sleep(5);
		kill(pid, SIGSTKFLT);
		break;
	case '6':
		break;
	case '7':
		// kill(pid, SIGINT);
		// kill(pid, SIGQUIT);
		// sleep(8);
		break;

	}
}
