#include "csapp.h"

int main() {
  char* buf;
  char* p;
  char content[MAXLINE];
  char query_str[MAXLINE];

  /* Extract the two arguments */
  if ((buf = getenv("QUERY_STRING")) != NULL) {
    p = strchr(buf, '&');
    *p = '\0';
    strcpy(query_str, buf);
  } 

  // Make response body
  sprintf(content, "The query string is: %s\r\n", query_str);

  // Make HTTP response
  printf("Content-length: %d\r\n", (int)strlen(content)); 
  printf("Content-type: text/plain\r\n\r\n"); 
  printf("%s", content);  // Add on response body from above
  fflush(stdout);

  return 0;
}