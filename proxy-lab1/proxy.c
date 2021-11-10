#include <stdio.h>
#include "csapp.h"
#include "sbuf.h"					// ADDED TO PROJ (also added sbuf.c)
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

// From http_parser
#define HTTP_REQUEST_MAX_SIZE 4096
#define HOSTNAME_MAX_SIZE 512
#define PORT_MAX_SIZE 6
#define URI_MAX_SIZE 4096
#define METHOD_SIZE 32

// For echoservert_pre.c thread
sbuf_t sbuf; /* Shared buffer of connected descriptors */

// For proxy.c - * You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
static const char *conn_hdr = "Connection: close\r\n";
static const char *proxy_conn_hdr = "Proxy-Connection: close\r\n";

char* get_client_request(int connfd) {
	// Reads everything from the file descriptor into the buffer
	char* buf = (char *) malloc(HTTP_REQUEST_MAX_SIZE);
	Read(connfd, buf, HTTP_REQUEST_MAX_SIZE);
	return buf;
}

// From http_parser
// Returns 1 if complete, 0 if incomplete
int is_complete_request(const char *request) {
	char *found_ptr = strstr(request, "\r\n\r\n");
	if (found_ptr != NULL) {
		return 1;
	}
	return 0;
}

// Based from http_parser
char* client_to_server_request(char *client_req) {
	char* server_req = (char*) malloc(HTTP_REQUEST_MAX_SIZE);

	// If client has not sent the full request, return 0 to show the request is not complete.
	if (is_complete_request(client_req) == 0) {
		return NULL;
	}

	// Make non-const request variable
	char temp_client_req[500];
	strcpy(temp_client_req, client_req);

	// strtok_r
	char* saveptr;	// Needed for strtok_r
	char* token = strtok_r(temp_client_req, "\r\n", &saveptr);
	
	while (token != NULL) {
		// If on first line
		if (strstr(token, "GET")) {
			strncat(server_req, token, strlen(token) - 1);	// Append first line except last one (shouldn't be 1.1, but 1.0)
			strcat(server_req, "0");	// 0 instead of 1 appended here so it's HTTP/1.0
			strcat(server_req, "\r\n");	// End line
		}

		// Host
		else if (strstr(token, "Host:")) {
			strcat(server_req, token);
			strcat(server_req, "\r\n");	// End line
		}

		// User Agent and other hard-coded tokens
		else if (strstr(token, "User-Agent:")) {
			strcat(server_req, user_agent_hdr);
			strcat(server_req, conn_hdr);
			strcat(server_req, proxy_conn_hdr);
		}

		// All other lines get appended to end
		else {
			strcat(server_req, token);
			strcat(server_req, "\r\n");	// End line
		}

		token = strtok_r(NULL, "\r\n", &saveptr);	// Move to next line
	}
	strcat(server_req, "\r\n");	// Add final \r\n to show the end of request

	return server_req;
}

// Based from echoservert_pre.c
void *thread(void *vargp) 
{  
	pthread_detach(pthread_self()); 
	while (1) { 
		int connfd = sbuf_remove(&sbuf); /* Remove connfd from buffer */ //line:conc:pre:removeconnfd
		char* client_req = get_client_request(connfd);
		char* server_req = client_to_server_request(client_req);
		if (server_req == NULL)
			perror("Invalid HTTP Request");
		printf("\nserver_req: %s\n", server_req);
		free(client_req);
		free(server_req);
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