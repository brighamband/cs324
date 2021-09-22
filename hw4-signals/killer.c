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
		// kill(pid, SIGHUP);
		// kill(pid, SIGINT);
		// sleep(12);
		break;
	case '5':
		break;
	case '6':
		break;
	case '7':
		break;

	}
}
