#include "csapp.h"
#include <stdio.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>


/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

#define MAX_EVENTS 25
#define MAX_HOST_SIZE 1024
#define MAX_PORT_SIZE 16

// State enums
#define STATE_READ_REQ 1
#define STATE_SEND_REQ 2
#define STATE_READ_RES 3
#define STATE_SEND_RES 4


/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

typedef struct {
    int client_socket_fd;
    int server_socket_fd;
    char buffer[MAX_OBJECT_SIZE];
    char host[MAX_HOST_SIZE];
    char port[MAX_PORT_SIZE];
    // FIXME -- Maybe store more data
    char server_request[MAX_OBJECT_SIZE];
    char server_response[MAX_OBJECT_SIZE];
    unsigned int state;
    unsigned int bytes_read_from_client;
    unsigned int bytes_to_write_server;
    unsigned int bytes_written_to_server;
    unsigned int bytes_written_to_client;
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
    memset(event->buffer, 0, MAX_OBJECT_SIZE);
    memset(event->host, 0, MAX_OBJECT_SIZE);
    memset(event->port, 0, MAX_OBJECT_SIZE);
    memset(event->server_request, 0, MAX_OBJECT_SIZE);
    memset(event->server_response, 0, MAX_OBJECT_SIZE);
    event->state = STATE_READ_REQ;
    event->bytes_read_from_client = 0;
    event->bytes_to_write_server = 0;
    event->bytes_written_to_server = 0;
    event->bytes_written_to_client = 0;
}

// 1.  Client -> Proxy
void read_request(event_data_t *event) {

    // use event->client_socket_fd

    // Parse http request (use code from before)
    // Call listen
    // Call accept
    // Loop while it's not \r\n\r\n
    // Call read, and pass in the fd you returned from calls above

    // set state to next state
    event->state = STATE_SEND_REQ;
}

// 2.  Proxy -> Server
void send_request(event_data_t *event) {
    // Call write to write the bytes received from client to the server

    // set state to next state
}

// 3.  Server -> Proxy
void read_response(event_data_t *event) {
    // use event->server_socket_fd

    // Loop while the return val from read is not 0
    // Call read

    int cur_read = 0;	// Reused, num bytes read in a single call 
	char* res_ptr = &event->server_response[0];
	int num_bytes_read = 0;

	while ((cur_read = Read(event->server_socket_fd, res_ptr, MAX_OBJECT_SIZE)) > 0) {
		res_ptr += cur_read;
		num_bytes_read += cur_read;
	}
	strcat(event->server_response, "\0");	// Denote end of string 

    // set state to next state
    event->state = STATE_SEND_RES;
}

// 4.  Proxy -> Client
void send_response(event_data_t *event) {
    // use event->server_socket_fd

    // Call write to write bytes received from server to the client

    // set state to next state
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

    printf("efd: %i\nlistenfd: %i\nevent.data.fd: %i\n", efd, listenfd, event.data.fd);

    /* Events buffer used by epoll_wait to list triggered events */
    events = (struct epoll_event*) calloc (MAX_EVENTS, sizeof(event));

    while(1) {
        printf("before epollwait\n");
        int num_events = epoll_wait(efd, events, MAX_EVENTS, -1);
        printf("num_events: %i", num_events);
        for (int i = 0; i < num_events; i++) {
            printf("in for\n");
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
