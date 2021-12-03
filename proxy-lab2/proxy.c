#include <stdio.h>
#include "csapp.h"
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netdb.h>

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
    char server_request[MAX_OBJECT_SIZE];
    unsigned int state;
    unsigned int bytes_read_from_client;
    unsigned int bytes_to_write_server;
    unsigned int bytes_written_to_server;
    unsigned int bytes_written_to_client;
} event_data_t;

void handle_new_connection(int efd, struct epoll_event *event) {
	struct sockaddr_in in_addr;
	int addr_size = sizeof(in_addr);
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
}

event_data_t* init_event_data(int efd/*, struct epoll_event *event*/) {
    event_data_t event_data = {
        .client_socket_fd = efd,
        .server_socket_fd = 0,
        .buffer = "",
        .host = "",
        .port = "",
        .server_request = "",
        .state = 0,
        .bytes_read_from_client = 0,
        .bytes_to_write_server = 0,
        .bytes_written_to_server = 0,
        .bytes_written_to_client = 0,
    };
    event_data_t *event_data_ptr = &event_data;
    return event_data_ptr;
}

// 1.  Client -> Proxy
void read_request() {
    // Call listen
    // Call accept
    // Loop while it's not \r\n\r\n
    // Call read, and pass in the fd you returned from calls above

    // set state to next state
}

// 2.  Proxy -> Server
void send_request() {
    // Call write to write the bytes received from client to the server

    // set state to next state
}

// 3.  Server -> Proxy
void read_response() {
    // Loop while the return val from read is not 0
    // Call read

    // set state to next state
}

// 4.  Proxy -> Client
void send_response() {
    // Call write to write bytes received from server to the client

    // set state to next state
}

int main(int argc, char **argv) {
    int efd, listenfd;
    struct epoll_event event, *events;
    struct client_info *active_event;

    // Return if bad arguments
    if (argc < 2) {
        printf("Usage: %s portnumber\n", argv[0]);
        exit(1);
    }

    // Create an epoll instance
    if(efd = epoll_create1(0) < 0) {
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
        int num_events = epoll_wait(efd, events, MAX_EVENTS, 1000);  // FIXME - is the 1000 right?  Milliseconds

        for (int i = 0; i < num_events; i++) {
            active_event = (struct client_info *)(events[i].data.ptr);

            // Skip over unneeded events
            if ((events[i].events & EPOLLERR) ||
					(events[i].events & EPOLLHUP) ||
					(events[i].events & EPOLLRDHUP)) {
				fprintf (stderr, "epoll error on %s\n", active_event->desc);
				close(active_event->fd);
				free(active_event);
				continue;
			}
        }


    }
    // infinite while
        // epoll_wait (returns num of events)
        // for n in events
            // active_event = state*
            // if ER, HUP, RDHUP
                // throw err
            // elif fd == listenfd
                // **** handle new connection (mostly copy)
                // initialize state struct (0s or empty for all except event->state = STATE_READ_REQ)
                // event_data_t *event_data_ptr = init_event_data(77);
                // register struct -- non-blocking
            // else
                // switch cases, active_event->state


    printf("%s", user_agent_hdr);

    //
    // else {
        // switch(states[state_idx].state) {
        //     read_request(efd, &states[states_idx]);
        // }
    // }
    return 0;
}
