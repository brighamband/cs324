/* Use this command to compile catmatch:  gcc catmatch.c -o catmatch (add '&& ./catmatch {filename}' to execute in one step) */

#include <stdio.h>
#include <unistd.h>

int printProcessId() {
  int pid = fork();

    if (pid == 0) {
    fprintf(stderr, "Process ID: %d\n", getpid());
  }
}

int main(int argc, char* argv []) {
  printProcessId();

  if (argc == 1) {
    puts("Make sure to specify a filename.");
    return 1;
  }

  FILE* file;
  char str[100];

  file = fopen(argv[1], "r");
  if (file == NULL) {
    perror("Error opening file");
    return 1;
  } 

  if (fgets(str, 100, file) != NULL) {
    puts(str);
  }
  fclose(file);

  return 0;
}

