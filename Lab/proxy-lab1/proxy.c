#include <stdio.h>

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

// START - ECHO THREAD SERVER CODE - echoservert.c
// Step 1
void client_req_to_proxy() {}

// Step 2
void proxy_req_to_server() {}    

// Step 3
void server_res_to_proxy() {}    

// Step 4
void proxy_res_to_client() {}    

int main() {
    
    // 1 - receive request from client
    // 2 - send off request to server

    // 3 - receive response from server
    // 4 - return response to client
    printf("%s", user_agent_hdr);
    return 0;
}
