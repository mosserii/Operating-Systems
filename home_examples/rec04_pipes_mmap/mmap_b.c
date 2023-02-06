#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#define FILEPATH "/tmp/mmapped.bin"
#define FILESIZE 4096

int main(int argc, char *argv[]) {
  int fd = open(FILEPATH, O_RDWR, 0777);
  if (fd == -1) {
    perror("Failed opening/reating mmap file");
    exit(EXIT_FAILURE);
  }

  int *x;
  x = (int *)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (x == MAP_FAILED) {
    perror("Failed mmaping");
    close(fd);
    exit(EXIT_FAILURE);
  }
  printf("%p\n", x);

  sleep(1);
  (*x)++;

  if (munmap((void *)x, sizeof(int)) == -1) {
    perror("Failed unmapping");
    close(fd);
    exit(EXIT_FAILURE);
  }

  close(fd);
}
