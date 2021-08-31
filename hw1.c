/* Use this command to compile catchmatch:  gcc hw1.c -o catchmatch (add '&& ./a.out' to execute in one step) */

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

  if (argv[1] == NULL) {
    puts("Make sure to specify a filename.");
    return 1;
  }
  puts("hello");

  printProcessId();

  return 0;
}

