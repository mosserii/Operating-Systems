#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#define FIFO_NAME "/tmp/osfifo"

int main(int argc, char *argv[]) {
  int fd = open(FIFO_NAME, O_WRONLY);
  if (fd == -1) {
    perror("Failed opening fifo");
    exit(EXIT_FAILURE);
  }

  char *msg = "Hello from fifo writer!";
  int msg_len = strlen(msg);
  ssize_t bytes_written;
  while ((bytes_written = write(fd, msg, msg_len)) > 0) {
    msg += bytes_written;
    msg_len -= bytes_written;
  }
  if (bytes_written == -1) {
    perror("Failed writing to fifo");
    close(fd);
    exit(EXIT_FAILURE);
  }

  close(fd);
}
