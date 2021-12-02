#include <stdio.h>

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

#define MAX_EVENTS 26 // FIXME - IS THIS THE RIGHT NUMBER
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
    int client_socket;
    int server_socket;
    char buffer[MAX_OBJECT_SIZE];
    char host[MAX_HOST_SIZE];
    char port[MAX_PORT_SIZE];
    char server_request[MAX_OBJECT_SIZE];
    unsigned int state;
    unsigned int bytes_read_from_client;
    unsigned int bytes_to_write_server;
    unsigned int bytes_written_to_server;
    unsigned int bytes_written_to_client;
} state_t;

state_t states[MAX_EVENTS];

// 1.  Client -> Proxy
void read_request() {
    // Call listen
    // Call accept
    // Loop while it's not \r\n\r\n
    // Call read, and pass in the fd you returned from calls above
}

// 2.  Proxy -> Server
void send_request() {
    // Call write to write the bytes received from client to the server
}

// 3.  Server -> Proxy
void read_response() {
    // Loop while the return val from read is not 0
    // Call read
}

// 4.  Proxy -> Client
void send_response() {
    // Call write to write bytes received from server to the client
}

int main()
{
    printf("%s", user_agent_hdr);

    //
    // else {
        // switch(states[state_idx].state) {
        //     read_request(efd, &states[states_idx]);
        // }
    // }
    return 0;
}
