/* Use this command to compile catchmatch:  gcc simple.c -o catchmatch (add '&& ./a.out' to execute in one step) */

#include <stdio.h>
#include <unistd.h>

int printProcessId() {
  int pid = fork();

    if (pid == 0) {
    printf("Parent Process id : %d\n", getpid());
    printf("Child Process with parent id : %d\n", getppid());
  }
}

int main() {
  puts("hello");

  printProcessId();

  return 0;
}

