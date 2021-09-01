/* Use this command to compile catmatch:  gcc catmatch.c -o catmatch (add '&& ./catmatch {filename}' to execute in one step) */
/* Must reset CATMATCH_PATTERN env var every session using export CATMATCH_PATTERN='linux' */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


int printProcessId() {
  int pid = fork();

    if (pid == 0) {
    fprintf(stderr, "Process ID: %d\n", getpid());
  }
}

int main(int argc, char* argv []) {
  if (argc == 1) {
    puts("Make sure to specify a filename.");
    return 1;
  }
  printProcessId();

  FILE* fp;
  char str[1024];

  fp = fopen(argv[1], "r");
  if (fp == NULL) {
    perror("Error opening file");
    return 1;
  }

  char* catmatchPattern = getenv("CATMATCH_PATTERN");

  while(fgets(str, sizeof str, fp) != NULL) {
    int marker = 0;
    if (catmatchPattern != NULL) {
      char* foundPtr = strstr(str, catmatchPattern);
      if (foundPtr != NULL) marker = 1;
    }
    printf("%d %s", marker, str);
  }
  printf("\n");
  fclose(fp);

  return 0;
}

