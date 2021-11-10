/* 
 * echoserveri.c - An iterative echo server 
 */ 
/* $begin echoserverimain */
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define MAXLINE 8192

void echo(int connfd);

int main(int argc, char **argv) 
{
	int listenfd, connfd;
	socklen_t clientlen;
	struct sockaddr_in ip4addr;
	struct sockaddr_storage clientaddr;  /* Enough space for any address */  //line:netp:echoserveri:sockaddrstorage
	char client_hostname[MAXLINE], client_port[MAXLINE];

	if (argc != 2) {
		fprintf(stderr, "usage: %s <port>\n", argv[0]);
		exit(0);
	}

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
		getnameinfo((struct sockaddr *) &clientaddr, clientlen, client_hostname, MAXLINE, 
				client_port, MAXLINE, 0);
		printf("Connected to (%s, %s)\n", client_hostname, client_port);
		echo(connfd);
		close(connfd);
	}
	exit(0);
}
/* $end echoserverimain */
