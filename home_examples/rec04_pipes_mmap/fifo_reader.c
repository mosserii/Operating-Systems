#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#define BUFF_SIZE 256
#define FIFO_NAME "/tmp/osfifo"

int main(int argc, char *argv[]) {
  if (mkfifo(FIFO_NAME, 0777) == -1) {
    perror("Failed making fifo");
    exit(EXIT_FAILURE);
  }

  int fd = open(FIFO_NAME, O_RDONLY);
  if (fd == -1) {
    perror("Failed opening fifo");
    unlink(FIFO_NAME);
    exit(EXIT_FAILURE);
  }

  char buff[BUFF_SIZE + 1];
  int bytes_read;
  while ((bytes_read = read(fd, buff, BUFF_SIZE)) > 0) {
    buff[bytes_read] = '\0';
    printf("%s", buff);
  }
  if (bytes_read == -1) {
    perror("Failed reading from fifo");
    close(fd);
    unlink(FIFO_NAME);
    exit(EXIT_FAILURE);
  }

  printf("\n");

  close(fd);
  unlink(FIFO_NAME);
}