#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
	int pid;

	printf("Starting program; process has pid %d\n", getpid());

	FILE * fp = fopen("fork-output.txt", "w");
	fprintf(fp, "BEFORE FORK\n");
	fflush(fp);

	if ((pid = fork()) < 0) {
		fprintf(stderr, "Could not fork()");
		exit(1);
	}

	/* BEGIN SECTION A */
	fprintf(fp, "SECTION A\n");
	fflush(fp);

	printf("Section A;  pid %d\n", getpid());
	sleep(30);

	/* END SECTION A */
	if (pid == 0) {
		/* BEGIN SECTION B */
		fprintf(fp, "SECTION B\n");
		fflush(fp);

		printf("Section B\n");
		// sleep(30);
		// sleep(30);
		printf("Section B done sleeping\n");

		exit(0);

		/* END SECTION B */
	} else {
		/* BEGIN SECTION C */
		fprintf(fp, "SECTION C\n");
		fflush(fp);

		printf("Section C\n");
		// sleep(30);
		wait(NULL);
		// sleep(30);
		printf("Section C done sleeping\n");

		exit(0);

		/* END SECTION C */
	}
	/* BEGIN SECTION D */
	fprintf(fp, "SECTION D\n");
	fflush(fp);

	printf("Section D\n");
	sleep(30);

	/* END SECTION D */
}
