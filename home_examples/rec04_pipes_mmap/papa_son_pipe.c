#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  int pipefd[2];
  pid_t cpid;

  if (2 != argc) {
    printf("Usage: %s <string>\n", argv[0]);
    exit(-1);
  }

  if (-1 == pipe(pipefd)) {
    perror("pipe");
    exit(-1);
  }

  cpid = fork();
  if (-1 == cpid) {
    perror("fork");
    close(pipefd[0]);
    close(pipefd[1]);
    exit(-1);
  }

  if (0 == cpid) {
    // Child reads from pipe
    // Close unused write end
    close(pipefd[1]);
    char buf;
    while (read(pipefd[0], &buf, 1) > 0) {
      printf("%c", buf);
    }
    puts("");
    close(pipefd[0]);
    exit(0);
  } else {
    // Parent writes argv[1] to pipe
    // Close unused read end
    close(pipefd[0]);

    // Write the data
    write(pipefd[1], argv[1], strlen(argv[1]));

    // Reader will see EOF
    close(pipefd[1]);

    // Wait for child
    wait(NULL);
    exit(0);
  }
}
