/*
 * Pipes are one of the simplest forms of IPC in Unix-like systems. They provide
 * a unidirectional communication channel between related processes, typically a
 * parent and child.
 *
 * Use pipes when:

You need simple, unidirectional communication between related processes
(typically parent-child). You're working with a stream of data rather than
discrete messages. Low latency is important for small amounts of data. You want
a lightweight solution with minimal setup.

Best scenarios:

Sending output from one process to another (like in shell pipelines).
Implementing producer-consumer patterns between parent and child processes.

Limitations:

Only works between related processes.
Unidirectional communication (you need two pipes for bidirectional).
Not suitable for complex data structures without additional serialization.



 * */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
int main(int argc, char *argv[]) {

  int pipefd[2];
  char buf[20];

  if (pipe(pipefd) == -1) {
    perror("pipe");
    return EXIT_FAILURE;
  }

  if (fork() == 0) {  // child process
    close(pipefd[1]); // pipe1 is used for writing, so don't need it here
    read(pipefd[0], buf, sizeof(buf));
    printf("Child recieved: %s\n", buf);
    close(pipefd[0]);
  } else {            // parent process
    close(pipefd[0]); // dont need to read
    write(pipefd[1], "Hello, child!", 14);
    close(pipefd[1]);
  }
  return EXIT_SUCCESS;
}
