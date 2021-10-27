/* 
 * echoservert_pre.c - A prethreaded concurrent echo server
 */
/* $begin echoservertpremain */
#include "sbuf.h"
#include "csapp.h"
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
#define NTHREADS  10
#define SBUFSIZE  20

void echo_cnt(int connfd);
void *thread(void *vargp);

sbuf_t sbuf; /* Shared buffer of connected descriptors */

int main(int argc, char **argv) 
{
	int i, listenfd, connfd;
	socklen_t clientlen;
	struct sockaddr_in ip4addr;
	struct sockaddr_storage clientaddr;
	pthread_t tid; 

	if (argc != 2) {
		fprintf(stderr, "usage: %s <port>\n", argv[0]);
		exit(0);
	}

	sbuf_init(&sbuf, SBUFSIZE); //line:conc:pre:initsbuf
	for (i = 0; i < NTHREADS; i++)  /* Create worker threads */ //line:conc:pre:begincreate
		pthread_create(&tid, NULL, thread, NULL);               //line:conc:pre:endcreate

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
		connfd = accept(listenfd, (struct sockaddr *) &clientaddr, &clientlen);
		sbuf_insert(&sbuf, connfd); /* Insert connfd in buffer */
	}
}

void *thread(void *vargp) 
{  
	pthread_detach(pthread_self()); 
	while (1) { 
		int connfd = sbuf_remove(&sbuf); /* Remove connfd from buffer */ //line:conc:pre:removeconnfd
		echo_cnt(connfd);                /* Service client */
		close(connfd);
	}
}
/* $end echoservertpremain */
