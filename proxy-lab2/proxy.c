#include "csapp.h"
#include <stdio.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

// curl -v --proxy http://localhost:23080 http://localhost:23081/home.html


/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

#define MAX_EVENTS 25
#define MAX_HOST_SIZE 1024
#define MAX_PORT_SIZE 16
#define MAX_METHOD_SIZE 32
#define MAX_URI_SIZE 4096


// State enums
#define STATE_READ_REQ 1
#define STATE_SEND_REQ 2
#define STATE_READ_RES 3
#define STATE_SEND_RES 4


/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
static const char *conn_hdr = "Connection: close\r\n";
static const char *proxy_conn_hdr = "Proxy-Connection: close\r\n";

typedef struct {
    int client_socket_fd;
    int server_socket_fd;
    // char buffer[MAX_OBJECT_SIZE];
    char host[MAX_HOST_SIZE];
    char port[MAX_PORT_SIZE];
    // FIXME -- Maybe store more data
    char client_request[MAX_OBJECT_SIZE];
    char server_request[MAX_OBJECT_SIZE];
    char server_response[MAX_OBJECT_SIZE];
    unsigned int state;
    unsigned int bytes_read_from_client;
    unsigned int bytes_to_write_server;
    unsigned int bytes_written_to_server;
    unsigned int bytes_written_to_client;
    // Added
    unsigned int bytes_read_from_server;
} event_data_t;

event_data_t events[MAX_EVENTS];

int connect_to_client(int efd, struct epoll_event *event) {
    printf("top of connect_to_client\n");
	struct sockaddr_in in_addr;
	unsigned int addr_size = sizeof(in_addr);
	char hbuf[MAXLINE], sbuf[MAXLINE];

    int connfd = Accept(event->data.fd, (struct sockaddr *)(&in_addr), &addr_size);

	/* get the client's IP addr and port num */
	int s = getnameinfo ((struct sockaddr *)&in_addr, addr_size,
                                   hbuf, sizeof hbuf,
                                   sbuf, sizeof sbuf,
                                   NI_NUMERICHOST | NI_NUMERICSERV);
	if (s == 0) {
	    printf("Accepted connection on descriptor %d (host=%s, port=%s)\n", connfd, hbuf, sbuf);
	}

    return connfd;
}

// Initialize event data for new event to be sent through proxy
void init_event_data(event_data_t *event, int connfd) {
    event->client_socket_fd = connfd;
    event->server_socket_fd = 0;
    // memset(event->buffer, 0, MAX_OBJECT_SIZE);
    memset(event->host, 0, MAX_OBJECT_SIZE);
    memset(event->port, 0, MAX_OBJECT_SIZE);
    memset(event->client_request, 0, MAX_OBJECT_SIZE);
    memset(event->server_request, 0, MAX_OBJECT_SIZE);
    memset(event->server_response, 0, MAX_OBJECT_SIZE);
    event->state = STATE_READ_REQ;
    event->bytes_read_from_client = 0;
    event->bytes_to_write_server = 0;
    event->bytes_written_to_server = 0;
    event->bytes_written_to_client = 0;
    // Added
    event->bytes_read_from_server = 0;
}

int is_complete_request(const char *request) {
	char *found_ptr = strstr(request, "\r\n\r\n");
	if (found_ptr != NULL) {
		return 1;
	}
	return 0;
}

void reformat_client_request(event_data_t *event) {
	// If client has not sent the full request, return 0 to show the request is not complete.
	if (is_complete_request(event->client_request) == 0) {
		return;
	}

	char* method = (char*) malloc(MAX_METHOD_SIZE);
	char* uri = (char*) malloc(MAX_URI_SIZE);

	/*
	 *		Parse request to get the method, hostname, port, uri
	 */
	
	// Make non-const request variable
	char temp_request[MAX_OBJECT_SIZE];
	strcpy(temp_request, event->client_request);

	// Grab method
	char* temp_ptr = strtok(temp_request, " ");
	strcpy(method, temp_ptr);

	// Grab entire path
	temp_ptr = temp_request + strlen(temp_ptr) + 1;  // Move past method
	temp_ptr += 7;	// Move past http://
	temp_ptr = strtok(temp_ptr, " ");

	// Grab everything before slash (Hostname and Port)
	char host_port_str[MAX_HOST_SIZE + MAX_PORT_SIZE];
	strcpy(host_port_str, temp_ptr);
	strtok(host_port_str, "/");

	// Host and Port
	char* port_str = strstr(host_port_str, ":");
	char host_str[MAX_HOST_SIZE];
	strcpy(host_str, host_port_str);

	// If port was specified
	if (port_str != NULL) {
		strtok(host_str, ":");
		strcpy(event->host, host_str);
		strcpy(event->port, port_str + 1);	// Copy port_str into port, skipping the : colon
	} 
	// If just hostname
	else {	// If not colon, make port the default
		strcpy(event->host, host_str);
		strcpy(event->port, "80");	// Default port number
	}

	// Grab everything after slash (URI), if there is a specific uri
	if (strstr(temp_ptr, "/")) {
		char* uri_str = temp_ptr + strlen(host_port_str) + 1;  // Make uri be everything past the host, port and slash (the +1)
		strcpy(uri, uri_str);
	}

	/*
	 *		Make new request
	 */

	// Concat first line
	strcat(event->server_request, method);
	strcat(event->server_request, " /"); // Space and slash for uri
	strcat(event->server_request, uri);
	strcat(event->server_request, " HTTP/1.0");	// Version
	strcat(event->server_request, "\r\n");	// End line

	// Concat second line
	strcat(event->server_request, "Host: ");
	strcat(event->server_request, event->host);
	strcat(event->server_request, ":");
	strcat(event->server_request, event->port);
	strcat(event->server_request, "\r\n");	// End line

	// Concat hard-coded headers
	strcat(event->server_request, user_agent_hdr);
	strcat(event->server_request, conn_hdr);
	strcat(event->server_request, proxy_conn_hdr);

	// Add final \r\n to signify end of file
	strcat(event->server_request, "\r\n");

	free(method);
	free(uri);
}

// 1.  Client -> Proxy
void read_request(event_data_t *event) {
    // use event->client_socket_fd

    // Parse http request (use code from before)
    // Call listen
    // Call accept

	int cur_read = 0;	// Reused, num bytes read in a single call 
	char* req_ptr = &event->client_request[0];

	while ((cur_read = Read(event->client_socket_fd, req_ptr, MAX_OBJECT_SIZE)) > 0) {	// Keeps going while still has bytes being read or until it's complete
		req_ptr += cur_read;

		if (is_complete_request(event->client_request))
			break;
	}
	strcat(event->client_request, "\0");	// Denote end of string 

	printf("client_req: %s\n", event->client_request);

    // Loop while it's not \r\n\r\n
    // Call read, and pass in the fd you returned from calls above

    // Convert to server_request
    reformat_client_request(event);

	printf("server_req: %s\n", event->server_request);


    // set state to next state
    event->state = STATE_SEND_REQ;
}

// 2.  Proxy -> Server
void send_request(event_data_t *event) {
    // Call write to write the bytes received from client to the server

    struct addrinfo hints;
	struct addrinfo *result, *rp;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;	// IPv4
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = 0;
	hints.ai_protocol = IPPROTO_TCP;	// TCP

	// Connect to server

	// Returns a list of address structures, so we try each address until we successfully connect
	Getaddrinfo(event->host, event->port, &hints, &result);

	for (rp = result; rp != NULL; rp = rp->ai_next) {
		event->server_socket_fd = socket(rp->ai_family, rp->ai_socktype,
				rp->ai_protocol);
		if (event->server_socket_fd == -1)
			continue;
		if (connect(event->server_socket_fd, rp->ai_addr, rp->ai_addrlen) != -1)
			break;                  /* Success */
		close(event->server_socket_fd);
	}
	if (rp == NULL) {               /* No address succeeded */
		fprintf(stderr, "Could not connect\n");
		exit(EXIT_FAILURE);
	}

	freeaddrinfo(result);           /* No longer needed */

    // Send request off to server

	char *str_ptr = &event->server_request[0];
	int chars_left = strlen(str_ptr);
	while (chars_left > 0) {
		int chars_written = Write(event->server_socket_fd, str_ptr, chars_left);
		chars_left -= chars_written;
		str_ptr += chars_written;
	}

    // set state to next state
    event->state = STATE_READ_RES;
}

// 3.  Server -> Proxy
void read_response(event_data_t *event) {
    // use event->server_socket_fd

    // Loop while the return val from read is not 0
    // Call read

    int cur_read = 0;	// Reused, num bytes read in a single call 
	// char* res_ptr = &event->server_response[0];

	while ((cur_read = Read(event->server_socket_fd, event->server_response + event->bytes_read_from_server, MAX_OBJECT_SIZE)) > 0) {
		// res_ptr += cur_read;
		event->bytes_read_from_server += cur_read;
	}
	strcat(event->server_response, "\0");	// Denote end of string 

    printf("Server response: %s\n", event->server_response);
    printf("Server response: %i\n", event->bytes_read_from_server);

	// set state to next state
	event->state = STATE_SEND_RES;
}

// 4.  Proxy -> Client
void send_response(event_data_t *event) {
	// use event->client_socket_fd

	// Call write to write bytes received from server to the client

	char *str_ptr = &event->server_response[0];
	int chars_left = event->bytes_read_from_server;
	while (chars_left > 0) {
		int chars_written = Write(event->client_socket_fd, str_ptr, chars_left);
		chars_left -= chars_written;
		str_ptr += chars_written;
	}

    // set state to next state MAY NOT BE NECESSARY FOR LAST ONE Maybe just need to ... 👇
    // Close file descriptor, close epoll instance

    Close(event->client_socket_fd);
    Close(event->server_socket_fd);
}

int main(int argc, char **argv) {
    int efd, listenfd, connfd;
    struct epoll_event event, *events;

    // Return if bad arguments
    if (argc < 2) {
        printf("Usage: %s portnumber\n", argv[0]);
        exit(1);
    }

    // Create an epoll instance
    if((efd = epoll_create1(0)) < 0) {
        printf("Unable to create epoll fd\n");
        exit(1);
    }

    // Set up listen socket
    listenfd = Open_listenfd(argv[1]);

    // Make listen socket non-blocking
    if (fcntl(listenfd, F_SETFL, fcntl(listenfd, F_GETFL, 0) | O_NONBLOCK) < 0) {
			fprintf(stderr, "error setting socket option\n");
			exit(1);
		}

    // Register listen socket with epoll instance for reading
    event.data.fd = listenfd;
    event.events = EPOLLIN | EPOLLET;
		if (epoll_ctl(efd, EPOLL_CTL_ADD, listenfd, &event) < 0) {
			fprintf(stderr, "error adding event\n");
			exit(1);
		}

    /* Events buffer used by epoll_wait to list triggered events */
    events = (struct epoll_event*) calloc (MAX_EVENTS, sizeof(event));

    while(1) {
        int num_events = epoll_wait(efd, events, MAX_EVENTS, -1);
        printf("num_events: %i\n", num_events);
        for (int i = 0; i < num_events; i++) {
            printf("inside for\n");
            event_data_t* active_event = (event_data_t *) events[i].data.ptr;

            // Skip over active event if ER, HUP, or RDHUP
            if ((events[i].events & EPOLLERR) ||
					(events[i].events & EPOLLHUP) ||
					(events[i].events & EPOLLRDHUP)) {
				fprintf (stderr, "epoll error\n");
				close(events[i].data.fd);
				free(active_event);
				continue;
			}

            // When client wants to connect to proxy
            else if (events[i].data.fd == listenfd) {
                connfd = connect_to_client(efd, &events[i]);

                // Initialize active event
                active_event = (event_data_t *) malloc(sizeof(event_data_t));
                init_event_data(active_event, connfd);

                // Register struct as non-blocking
                int flags = fcntl (connfd, F_GETFL, 0);
                flags |= O_NONBLOCK;
                fcntl (connfd, F_SETFL, flags);

                event.data.ptr = active_event;
                event.events = EPOLLIN | EPOLLET;
                if (epoll_ctl(efd, EPOLL_CTL_ADD, connfd, &event) < 0) {
                    fprintf(stderr, "error adding event\n");
                    exit(1);
                }            
            }

            // Every other type of connection
            else {
                switch (active_event->state) {
                    // 1.  Client -> Proxy
                    case STATE_READ_REQ:
                        read_request(active_event);
                        // break;

                    // 2.  Proxy -> Server
                    case STATE_SEND_REQ:
                        send_request(active_event);
                        // break;

                    // 3.  Server -> Proxy
                    case STATE_READ_RES:
                        read_response(active_event);
                        // break;

                    // 4.  Proxy -> Client
                    case STATE_SEND_RES:
                        send_response(active_event);
                        // break;

                    default:
                        break;
                }
            }
        }


    }

    printf("%s", user_agent_hdr);

    return 0;
}