#include <stdio.h>
#include "csapp.h"
#include "sbuf.h"					// ADDED TO PROJ (also added sbuf.c)
#include "http_parser.c"	// ADDED TO PROJ
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

// From echoservert_pre.c
#define MAXLINE 8192
#define NTHREADS  10
#define SBUFSIZE  20

// For proxy.c - /* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

// For echoservert_pre.c thread
sbuf_t sbuf; /* Shared buffer of connected descriptors */

// For proxy.c - * You won't lose style points for including this long line in your code */
// static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

char* get_client_request(int connfd) {
	// Reads everything from the file descriptor into the buffer
	char* buf = (char *) malloc(HTTP_REQUEST_MAX_SIZE);
	Read(connfd, buf, HTTP_REQUEST_MAX_SIZE);
	return buf;
}

// Based from echoservert_pre.c
void *thread(void *vargp) 
{  
	pthread_detach(pthread_self()); 
	while (1) { 
		int connfd = sbuf_remove(&sbuf); /* Remove connfd from buffer */ //line:conc:pre:removeconnfd
		// echo_cnt(connfd);                /* Service client */
		char* req = get_client_request(connfd);
		parse_client_request(req);
		free(req);
		close(connfd);
	}
}

// // Step 1
// void client_req_to_proxy() {}

// // Step 2
// void proxy_req_to_server() {}    

// // Step 3
// void server_res_to_proxy() {}    

// // Step 4
// void proxy_res_to_client() {}  

// Based off echoservert_pre.c
int main(int argc, char **argv) {
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
    
    // 1 - receive request from client
    // 2 - send off request to server

    // 3 - receive response from server
    // 4 - return response to client
    // printf("%s", user_agent_hdr);
    return 0;
}