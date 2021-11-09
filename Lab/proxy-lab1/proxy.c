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

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
// static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

// START - ECHO THREAD SERVER CODE - echoservert.c
// Step 1
void client_req_to_proxy() {}

// Step 2
void proxy_req_to_server() {}    

// Step 3
void server_res_to_proxy() {}    

// Step 4
void proxy_res_to_client() {}   

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

    // 1 - receive request from client
    // 2 - send off request to server

    // 3 - receive response from server
    // 4 - return response to client
    // printf("%s", user_agent_hdr);
}

/*
 * echo - read and echo text lines until client closes connection
 */
/* $begin echo */
#include "csapp.h"

void echo(int connfd) 
{
    size_t n; 
    char buf[MAXLINE]; 
    rio_t rio;

    Rio_readinitb(&rio, connfd);
    while((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) { //line:netp:echo:eof
	printf("server received %d bytes\n", (int)n);
	Rio_writen(connfd, buf, n);
    }
}
/* $end echo */

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
