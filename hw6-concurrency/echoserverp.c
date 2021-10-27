/* 
 * echoserverp.c - A concurrent echo server based on processes
 */
/* $begin echoserverpmain */
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAXLINE 8192

void echo(int connfd);

void sigchld_handler(int sig) //line:conc:echoserverp:handlerstart
{
	while (waitpid(-1, 0, WNOHANG) > 0)
		;
	return;
} //line:conc:echoserverp:handlerend

int main(int argc, char **argv) 
{
	int listenfd, connfd;
	socklen_t clientlen;
	struct sockaddr_in ip4addr;
	struct sockaddr_storage clientaddr;

	if (argc != 2) {
		fprintf(stderr, "usage: %s <port>\n", argv[0]);
		exit(0);
	}

	signal(SIGCHLD, sigchld_handler);

	ip4addr.sin_family = AF_INET;
	ip4addr.sin_port = htons(atoi(argv[1]));
	ip4addr.sin_addr.s_addr = INADDR_ANY;
	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket error");
		exit(EXIT_FAILURE);
	}
	if (bind(listenfd, (struct sockaddr*)&ip4addr, sizeof(struct sockaddr_in)) < 0) {
		close(listenfd);
		perror("bind error");
		exit(EXIT_FAILURE);
	}
	if (listen(listenfd, 100) < 0) {
		close(listenfd);
		perror("listen error");
		exit(EXIT_FAILURE);
	} 

	while (1) {
		clientlen = sizeof(struct sockaddr_storage); 
		connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);
		if (fork() == 0) { 
			close(listenfd); /* Child closes its listening socket */
			echo(connfd);    /* Child services client */ //line:conc:echoserverp:echofun
			close(connfd);   /* Child closes connection with client */ //line:conc:echoserverp:childclose
			exit(0);         /* Child exits */
		}
		close(connfd); /* Parent closes connected socket (important!) */ //line:conc:echoserverp:parentclose
	}
}
/* $end echoserverpmain */
