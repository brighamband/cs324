#include "csapp.h"

int main() {
  // Get query from env
  char *query_str = getenv("QUERY_STRING");

  // Make HTTP response
  // Include response headers
  printf("Content-length: %d\r\n", (int)strlen(query_str)); 
  printf("Content-type: text/plain\r\n\r\n");
  // Include response body
  printf("The query string is: %s\n", query_str);

  return 0;
}