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

