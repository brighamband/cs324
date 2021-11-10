#include "csapp.h"

int main() {
  // Get query from env
  char *query_str = getenv("QUERY_STRING");

  // Make HTTP response
  // Include response headers
  char *body = "The query string is: ";
  printf("Content-length: %d\r\n", (int) strlen(body) + (int) strlen(query_str)); 
  printf("Content-type: text/plain\r\n\r\n");
  // Include response body
  printf("%s%s\n", body, query_str);

  return 0;
}