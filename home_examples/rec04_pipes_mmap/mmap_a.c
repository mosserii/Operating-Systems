#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#define FILEPATH "/tmp/mmapped.bin"
#define FILESIZE 4096

int main(int argc, char *argv[]) {
  int fd = open(FILEPATH, O_RDWR | O_CREAT, 0777);
  if (fd == -1) {
    perror("Failed opening/reating mmap file");
    exit(EXIT_FAILURE);
  }

  if (lseek(fd, FILESIZE - 1, SEEK_SET) == -1 || write(fd, "\0", 1) == -1) {
    perror("Failed initializing mmap file");
    close(fd);
    exit(EXIT_FAILURE);
  }

  int pid = fork();
  if (pid == -1) {
    perror("Failed forking");
    exit(EXIT_FAILURE);
  } else if (pid == 0) {
    // Child
    char *arg_list[] = {"./mmap_b.out", NULL};
    execvp(arg_list[0], arg_list);
    perror("Failed executing mmap_b");
    exit(EXIT_FAILURE);
  } else {
    // Parent
    int *x;
    x = (int *)mmap(NULL, FILESIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (x == MAP_FAILED) {
      perror("Failed mmaping");
      close(fd);
      exit(EXIT_FAILURE);
    }
    printf("%p\n", x);

    *x = 5;
    printf("Value of x in mmap_a is %d\n", *x);
    sleep(2);
    printf("Value of x in mmap_a is %d\n", *x);

    if (munmap(x, FILESIZE) == -1) {
      perror("Failed unmapping");
      close(fd);
      exit(EXIT_FAILURE);
    }

    close(fd);
  }
}
