#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUF_SIZE 256

int main(int argc, char *argv[]) {
  int p2s_fd[2];
  int s2p_fd[2];
  pid_t cpid;
  char buf[BUF_SIZE];
  ssize_t n_bytes = -1;

  pipe(p2s_fd);
  pipe(s2p_fd);
  cpid = fork();

  if (cpid == 0) {
    // Child
    close(p2s_fd[1]);
    close(s2p_fd[0]);

    n_bytes = read(p2s_fd[0], buf, BUF_SIZE);
    buf[n_bytes] = '\0';
    printf("Child> %s\n", buf);
    close(p2s_fd[0]);

    sprintf(buf, "Child read %zd bytes", n_bytes);
    n_bytes = strlen(buf);
    write(s2p_fd[1], buf, n_bytes);
    close(s2p_fd[1]);
    exit(EXIT_SUCCESS);
  } else {
    // Parent
    close(p2s_fd[0]);
    close(s2p_fd[1]);

    write(p2s_fd[1], argv[1], strlen(argv[1]));
    close(p2s_fd[1]);

    n_bytes = read(s2p_fd[0], buf, BUF_SIZE);
    buf[n_bytes] = '\0';
    close(s2p_fd[0]);
    printf("Parent> %s\n", buf);
    // Wait for child
    wait(NULL);
    exit(EXIT_SUCCESS);
  }
}
