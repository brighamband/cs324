/* Use this command to compile catmatch:  gcc catmatch.c -o catmatch (add '&& ./catmatch {filename}' to execute in one step) */

#include <stdio.h>
#include <unistd.h>

int printProcessId() {
  int pid = fork();

    if (pid == 0) {
    printf("Parent Process id : %d\n", getpid());
    printf("Child Process with parent id : %d\n", getppid());
  }
}

int main(int argc, char* argv []) {
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

  // printf("string: %s", str);
  puts("");
  printProcessId();

  return 0;
}

