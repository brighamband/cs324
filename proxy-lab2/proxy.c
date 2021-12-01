#include <stdio.h>

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

void client_to_proxy() {
    // Call listen
    // Call accept
    // Loop while it's not \r\n\r\n
    // Call read, and pass in the fd you returned from calls above
}

void proxy_to_server() {
    // Call write to write the bytes received from client to the server
}

void server_to_proxy() {
    // Loop while the return val from read is not 0
    // Call read
}

void proxy_to_client() {
    // Call write to write bytes received from server to the client
}

int main()
{
    printf("%s", user_agent_hdr);
    return 0;
}
