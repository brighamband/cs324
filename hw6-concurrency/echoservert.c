/* 
 * echoservert.c - A concurrent echo server using threads
 */
/* $begin echoservertmain */
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#define MAXLINE 8192

void echo(int connfd);
void *thread(void *vargp);

int main(int argc, char **argv) 
{
	int listenfd, *connfdp;
	socklen_t clientlen;
	struct sockaddr_in ip4addr;
	struct sockaddr_storage clientaddr;
	pthread_t tid; 

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
		connfdp = malloc(sizeof(int)); //line:conc:echoservert:beginmalloc
		*connfdp = accept(listenfd, (struct sockaddr *) &clientaddr, &clientlen); //line:conc:echoservert:endmalloc
		pthread_create(&tid, NULL, thread, connfdp);
	}
}

/* Thread routine */
void *thread(void *vargp) 
{  
	int connfd = *((int *)vargp);
	pthread_detach(pthread_self()); //line:conc:echoservert:detach
	free(vargp);                    //line:conc:echoservert:free
	echo(connfd);
	close(connfd);
	return NULL;
}
/* $end echoservertmain */
