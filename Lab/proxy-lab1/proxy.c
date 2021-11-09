#include <stdio.h>
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

/*** sbuf.h - START **************************************************************************/
typedef struct {
    int *buf;          /* Buffer array */         
    int n;             /* Maximum number of slots */
    int front;         /* buf[(front+1)%n] is first item */
    int rear;          /* buf[rear%n] is last item */
    sem_t mutex;       /* Protects accesses to buf */
    sem_t slots;       /* Counts available slots */
    sem_t items;       /* Counts available items */
} sbuf_t;

void sbuf_init(sbuf_t *sp, int n)
{
    sp->buf = calloc(n, sizeof(int)); 
    sp->n = n;                       /* Buffer holds max of n items */
    sp->front = sp->rear = 0;        /* Empty buffer iff front == rear */
    sem_init(&sp->mutex, 0, 1);      /* Binary semaphore for locking */
    sem_init(&sp->slots, 0, n);      /* Initially, buf has n empty slots */
    sem_init(&sp->items, 0, 0);      /* Initially, buf has zero data items */
}

void sbuf_deinit(sbuf_t *sp)
{
    free(sp->buf);
}

void sbuf_insert(sbuf_t *sp, int item)
{
    sem_wait(&sp->slots);                          /* Wait for available slot */
    sem_wait(&sp->mutex);                          /* Lock the buffer */
    sp->buf[(++sp->rear)%(sp->n)] = item;   /* Insert the item */
    sem_post(&sp->mutex);                          /* Unlock the buffer */
    sem_post(&sp->items);                          /* Announce available item */
}

int sbuf_remove(sbuf_t *sp)
{
    int item;
    sem_wait(&sp->items);                          /* Wait for available item */
    sem_wait(&sp->mutex);                          /* Lock the buffer */
    item = sp->buf[(++sp->front)%(sp->n)];  /* Remove the item */
    sem_post(&sp->mutex);                          /* Unlock the buffer */
    sem_post(&sp->slots);                          /* Announce available slot */
    return item;
}
/*** sbuf.h - END ****************************************************************************/

/*** echo_cnt.c - START **************************************************************************/
static int byte_cnt;  /* Byte counter */
static sem_t mutex;   /* and the mutex that protects it */

static void init_echo_cnt(void)
{
    Sem_init(&mutex, 0, 1);
    byte_cnt = 0;
}

void echo_cnt(int connfd) 
{
    int n; 
    char buf[MAXLINE]; 
    rio_t rio;
    static pthread_once_t once = PTHREAD_ONCE_INIT;

    Pthread_once(&once, init_echo_cnt); //line:conc:pre:pthreadonce
    Rio_readinitb(&rio, connfd);        //line:conc:pre:rioinitb
    while((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
	P(&mutex);
	byte_cnt += n; //line:conc:pre:cntaccess1
	printf("server received %d (%d total) bytes on fd %d\n", 
	       n, byte_cnt, connfd); //line:conc:pre:cntaccess2
	V(&mutex);
	Rio_writen(connfd, buf, n);
    }
}
/*** echo_cnt.c - END ****************************************************************************/

/*** echoservert_pre.c - START **************************************************************************/
#define MAXLINE 8192
#define NTHREADS  10
#define SBUFSIZE  20

sbuf_t sbuf; /* Shared buffer of connected descriptors */

void *thread(void *vargp) 
{  
	pthread_detach(pthread_self()); 
	while (1) { 
		int connfd = sbuf_remove(&sbuf); /* Remove connfd from buffer */ //line:conc:pre:removeconnfd
		echo_cnt(connfd);                /* Service client */
		close(connfd);
	}
}
/*** echoservert_pre.c - END ****************************************************************************/

/*** proxy.c - START **************************************************************************/

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
// static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

// START - ECHO THREAD SERVER CODE - echoservert.c
// Step 1
void client_req_to_proxy() {}

// Step 2
void proxy_req_to_server() {}    

// Step 3
void server_res_to_proxy() {}    

// Step 4
void proxy_res_to_client() {}    
/*** proxy.c - END ****************************************************************************/

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