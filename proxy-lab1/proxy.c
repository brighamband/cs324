#include <stdio.h>
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
#include <semaphore.h>

// From echoservert_pre.c
#define MAXLINE 8192
#define NTHREADS  10
#define SBUFSIZE  20

// From hw6
#define HTTP_REQUEST_MAX_SIZE 4096
#define HOSTNAME_MAX_SIZE 512
#define PORT_MAX_SIZE 6
#define URI_MAX_SIZE 4096
#define METHOD_SIZE 32

// For proxy.c - /* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

// For echoservert_pre.c thread
sbuf_t sbuf; /* Shared buffer of connected descriptors */

// For proxy.c - * You won't lose style points for including this long line in your code */
// static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

char* get_http_request(int connfd) {
	// Reads everything from the file descriptor into the buffer
	char* buf = (char *) malloc(HTTP_REQUEST_MAX_SIZE);
	Read(connfd, buf, HTTP_REQUEST_MAX_SIZE);
	return buf;
}

// From hw6
int is_complete_request(char *request) {
	char *found_ptr = strstr(request, "\r\n\r\n");
	if (found_ptr != NULL) {
		return 1;
	}
	return 0;
}

// Based off hw6 but customized
char* parse_http_request(char *request) {
	char method[METHOD_SIZE];
	char hostname[HOSTNAME_MAX_SIZE];
	char port[PORT_MAX_SIZE];
	char uri[URI_MAX_SIZE];
	char* saveptr;

	// If client has not sent the full request, return 0 to show the request is not complete.
	if (is_complete_request(request) == 0) {
		return NULL;
	}

	// Make non-const request variable
	char temp_request[500];
	strcpy(temp_request, request);

	char* token = strtok_r(temp_request, "\r\n", &saveptr);
	
	while (token != NULL) {
		printf("TOKEN: %s\n", token);

		// If on first line
		if (strstr(token, "GET")) {
			// Method
			strcpy(method, "GET");

			// URI
			token = token + strlen(method) + sizeof(char);	// Move past method and space
			size_t len = strcspn(token, " ");
			strncpy(uri, token, len);
		}

		// Host
		if (strstr(token, "Host:")) {
			token += strlen("Host: ");	// Skip past Host and space
			strcpy(hostname, token);
		}

		token = strtok_r(NULL, "\r\n", &saveptr);
	}
	// // Grab method
	// char* temp_ptr = strtok(temp_request, " ");
	// strcpy(method, temp_ptr);

	// // Grab uri
	// temp_ptr = temp_request + strlen(temp_ptr) + 1;
	// strtok(temp_ptr, " ");
	// strcpy(uri, temp_ptr);

	// // Host
	// temp_ptr = temp_request + strlen(temp_ptr);
	// strtok(temp_ptr, " ");
	// strtok(temp_ptr, " ");
	// strcpy(hostname, temp_ptr);

	// Grab entire path
	// temp_ptr = temp_request + strlen(temp_ptr) + 1;  // Move past method
	// temp_ptr += 7;	// Move past http://
	// temp_ptr = strtok(temp_ptr, " ");

	// // Grab everything before slash (Hostname and Port)
	// char host_port_str[500];
	// strcpy(host_port_str, temp_ptr);
	// strtok(host_port_str, "/");

	// // Host and Port
	// char* port_str = strstr(host_port_str, ":");
	// char host_str[500];
	// strcpy(host_str, host_port_str);

	// // If port was specified
	// if (port_str != NULL) {
	// 	strtok(host_str, ":");
	// 	strcpy(hostname, host_str);
	// 	strcpy(port, port_str + 1);	// Copy port_str into port, skipping the : colon
	// } 
	// // If just hostname
	// else {	// If not colon, make port the default
	// 	strcpy(hostname, host_str);
	// 	strcpy(port, "80");	// Default port number
	// }

	// // Grab everything after slash (URI), if there is a specific uri
	// if (strstr(temp_ptr, "/")) {
	// 	char* uri_str = temp_ptr + strlen(host_port_str) + 1;  // Make uri be everything past the host, port and slash (the +1)
	// 	strcpy(uri, uri_str);
	// }

	printf("\nrequest: %s\n", request);
	printf("method: %s\n", method);
	printf("hostname: %s\n", hostname);
	printf("port: %s\n", port);
	printf("uri: %s\n", uri);

	return NULL;	// Return 1 string that's the request
}

// Based from echoservert_pre.c
void *thread(void *vargp) 
{  
	pthread_detach(pthread_self()); 
	while (1) { 
		int connfd = sbuf_remove(&sbuf); /* Remove connfd from buffer */ //line:conc:pre:removeconnfd
		// echo_cnt(connfd);                /* Service client */
		char* req = get_http_request(connfd);
		parse_http_request(req);
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
/*** echoservert_pre.c - START **************************************************************************/