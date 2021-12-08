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

// Socket enums
#define READING 1
#define WRITING 2


/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
static const char *conn_hdr = "Connection: close\r\n";
static const char *proxy_conn_hdr = "Proxy-Connection: close\r\n";

typedef struct {
    int client_socket_fd;
    int server_socket_fd;
    char host[MAX_HOST_SIZE];
    char port[MAX_PORT_SIZE];
    char client_request[MAX_OBJECT_SIZE];
    char server_request[MAX_OBJECT_SIZE];
    char server_response[MAX_OBJECT_SIZE];
    unsigned int state;
    unsigned int bytes_written_to_server;
    unsigned int bytes_read_from_server; 
    unsigned int bytes_written_to_client;
} event_data_t;

event_data_t events[MAX_EVENTS];

void make_socket_nonblocking(int fd) {
    if (fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK) < 0) {
        unix_error("error setting socket option\n");
    }
}

void setup_socket_for_epoll(int efd, struct epoll_event *ev, int socket_fd, int read_or_write, int add_or_mod) {
    ev->data.fd = socket_fd;

    if (read_or_write == READING)
        ev->events = EPOLLIN | EPOLLET;
    else if (read_or_write == WRITING)
        ev->events = EPOLLOUT;

    if (epoll_ctl(efd, add_or_mod, socket_fd, ev) < 0) {
        fprintf(stderr, "error adding event\n");
        exit(1);
    }
}

void connect_to_client(event_data_t *event, int efd, struct epoll_event *ev) {
	struct sockaddr_in in_addr;
	unsigned int addr_size = sizeof(in_addr);
	char hbuf[MAXLINE], sbuf[MAXLINE];

    int connfd = Accept(ev->data.fd, (struct sockaddr *)(&in_addr), &addr_size);

	/* get the client's IP addr and port num */
	int s = getnameinfo ((struct sockaddr *)&in_addr, addr_size,
                                   hbuf, sizeof hbuf,
                                   sbuf, sizeof sbuf,
                                   NI_NUMERICHOST | NI_NUMERICSERV);
	if (s == 0) {
	    printf("Accepted client connection on descriptor %d (host=%s, port=%s)\n", connfd, hbuf, sbuf);
	}

    // Set event's client socket fd to be the one returned from accept
    event->client_socket_fd = connfd;
}

void connect_to_server(event_data_t *event) {
    struct addrinfo hints;
	struct addrinfo *result, *rp;
	int sfd;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;	// IPv4
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = 0;
	hints.ai_protocol = IPPROTO_TCP;	// TCP

	// Connect to server

	// Returns a list of address structures, so we try each address until we successfully connect
	Getaddrinfo(event->host, event->port, &hints, &result);

	for (rp = result; rp != NULL; rp = rp->ai_next) {
		sfd = socket(rp->ai_family, rp->ai_socktype,
				rp->ai_protocol);
		if (sfd == -1)
			continue;
		if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
            printf("Accepted server connection on descriptor %d (host=%s, port=%s)\n", sfd, event->host, event->port);
			break;                  /* Success */
		close(sfd);
	}
	if (rp == NULL) {               /* No address succeeded */
		fprintf(stderr, "Could not connect\n");
		exit(EXIT_FAILURE);
	}

	freeaddrinfo(result);           /* No longer needed */

    // Set event's server socket fd to be the one returned from connect
    event->server_socket_fd = sfd;
}

// Initialize event data for new event to be sent through proxy
void init_event_data(event_data_t *event) {
    event->client_socket_fd = 0;
    event->server_socket_fd = 0;
    memset(event->host, 0, MAX_OBJECT_SIZE);
    memset(event->port, 0, MAX_OBJECT_SIZE);
    memset(event->client_request, 0, MAX_OBJECT_SIZE);
    memset(event->server_request, 0, MAX_OBJECT_SIZE);
    memset(event->server_response, 0, MAX_OBJECT_SIZE);
    event->state = STATE_READ_REQ;
    event->bytes_written_to_server = 0;
    event->bytes_read_from_server = 0;
    event->bytes_written_to_client = 0;
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
void read_request(event_data_t *event, int efd, struct epoll_event *ev) {
    printf("read_request()\n");

    // Loop while it's not \r\n\r\n
    // Call read, and pass in the fd you returned from calls above
	int cur_read = 0;
	while ((cur_read = Read(event->client_socket_fd, event->client_request + cur_read, MAX_OBJECT_SIZE)) > 0) {	// Keeps going while still has bytes being read or until it's complete
		if (is_complete_request(event->client_request))
			break;
	}
	strcat(event->client_request, "\0");	// Denote end of string 

	printf("client_req: %s\n", event->client_request);

    // Convert to server_request
    reformat_client_request(event);

	printf("server_req: %s\n", event->server_request);

    // Set up a new socket for server
    connect_to_server(event);

    // Configure server socket as non-blocking
    make_socket_nonblocking(event->server_socket_fd);

    // Register the socket w/ epoll instance for writing
    setup_socket_for_epoll(efd, ev, event->server_socket_fd, WRITING, EPOLL_CTL_ADD);

    // Set state to next state
    event->state = STATE_SEND_REQ;
}

// 2.  Proxy -> Server
void send_request(event_data_t *event, int efd, struct epoll_event *ev) {
    printf("send_request()\n");

    // Call write to write the bytes received from client to the server
	int chars_left = strlen(event->server_request);
    int chars_written = 0;
	while ((chars_written = Write(event->server_socket_fd, event->server_request + event->bytes_written_to_server, chars_left)) > 0) {
		event->bytes_written_to_server += chars_written;
        chars_left -= chars_written;
	}

    // Register the socket with the epoll instance for reading
    setup_socket_for_epoll(efd, ev, event->server_socket_fd, READING, EPOLL_CTL_MOD);

    // Set state to next state
    event->state = STATE_READ_RES;
}

// 3.  Server -> Proxy
void read_response(event_data_t *event, int efd, struct epoll_event *ev) {
    printf("read_response()\n");

    // Loop while the return val from read is not 0
    int cur_read = 0;
	while ((cur_read = Read(event->server_socket_fd, event->server_response + event->bytes_read_from_server, MAX_OBJECT_SIZE)) > 0) {
		event->bytes_read_from_server += cur_read;
	}
	strcat(event->server_response, "\0");	// Denote end of string 

    printf("Server response: %s\n", event->server_response);
    printf("Bytes read from server: %i\n", event->bytes_read_from_server);

    // Register the client socket with the epoll instance for writing
    setup_socket_for_epoll(efd, ev, event->client_socket_fd, WRITING, EPOLL_CTL_MOD);

	// Set state to next state
	event->state = STATE_SEND_RES;
}

// 4.  Proxy -> Client
void send_response(event_data_t *event, int efd, struct epoll_event *ev) {
    printf("send_response()\n");

	// Call write to write bytes received from server to the client
	int chars_left = event->bytes_read_from_server;
    int chars_written = 0;
	while ((chars_written = Write(event->client_socket_fd, event->server_response + event->bytes_written_to_client, chars_left)) > 0) {
        event->bytes_written_to_client += chars_written;
		chars_left -= chars_written;
	}

    // Close file descriptors, close epoll instance
    Close(event->client_socket_fd);
    Close(event->server_socket_fd);
}

int main(int argc, char **argv) {
    int efd, listenfd;
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
    make_socket_nonblocking(listenfd);

    // Register listen socket with epoll instance for reading
    setup_socket_for_epoll(efd, &event, listenfd, READING, EPOLL_CTL_ADD);

    /* Events buffer used by epoll_wait to list triggered events */
    events = (struct epoll_event*) calloc (MAX_EVENTS, sizeof(event));

    while(1) {
        int num_events = epoll_wait(efd, events, MAX_EVENTS, -1);
        printf("num_events: %i\n", num_events);
        for (int i = 0; i < num_events; i++) {
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

                // Initialize active event
                active_event = (event_data_t *) malloc(sizeof(event_data_t));
                init_event_data(active_event);

                // Connect to client
                connect_to_client(active_event, efd, &events[i]);

                // Register struct as non-blocking
                int flags = fcntl (active_event->client_socket_fd, F_GETFL, 0);
                flags |= O_NONBLOCK;
                fcntl (active_event->client_socket_fd, F_SETFL, flags);

                // Register client socket for reading for first time
                event.data.ptr = active_event;  // Also set the ptr to be the active event
                setup_socket_for_epoll(efd, &event, active_event->client_socket_fd, READING, EPOLL_CTL_ADD);  

                printf("active_event->state in else if: %i\n", active_event->state);    
            }

            // Every other type of connection
            else {
                printf("got dito\n");    
                printf("active_event->state in else: %i\n", active_event->state);    
                switch (active_event->state) {
                    // 1.  Client -> Proxy
                    case STATE_READ_REQ:
                        read_request(active_event, efd , &events[i]);
                        break;

                    // 2.  Proxy -> Server
                    case STATE_SEND_REQ:
                        send_request(active_event, efd , &events[i]);
                        break;

                    // 3.  Server -> Proxy
                    case STATE_READ_RES:
                        read_response(active_event, efd , &events[i]);
                        break;

                    // 4.  Proxy -> Client
                    case STATE_SEND_RES:
                        send_response(active_event, efd , &events[i]);
                        break;
                }
            }
        }


    }

    // Cleanup

    return 0;
}