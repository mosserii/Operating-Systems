#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define FILEPATH "/tmp/mmapped.bin"
#define FILESIZE 4000

int main(int argc, char *argv[]) {
  int fd;

  // open a file for writing.
  // Note: read/write mode needs to match
  // required access in mmap (not intuitive)
  fd = open(FILEPATH, O_RDWR | O_CREAT, 0644);
  if (-1 == fd) {
    printf("Error opening file for "
           "writing: %s\n",
           strerror(errno));
    return -1;
  }

  // Stretch the file size to the size
  // of the (mmapped) array
  if (lseek(fd, FILESIZE - 1, SEEK_SET) == -1) {
    printf("Error calling lseek() to 'stretch' "
           "the file: %s\n",
           strerror(errno));
    return -1;
  }

  // Something needs to be written at the end
  // of the file to have the file actually have
  // the new size.
  if (write(fd, "", 1) != 1) {
    printf("Error writing last byte of the "
           "file: %s\n",
           strerror(errno));
    return -1;
  }

  // Now the file is ready to be mmapped.
  char* arr = (char *)mmap(NULL, FILESIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (MAP_FAILED == arr) {
    printf("Error mmapping the file: %s\n", strerror(errno));
    return -1;
  }

  // now write to the file as if it were memory
  for (size_t i = 0; i < FILESIZE; ++i) {
    arr[i] = 'a';
    // if we wanted to be sure the changes
    // commit immediately to file
    // we should have msynced here (with MS_SYNC)
  }

  // don't forget to free the mmapped memory
  // this also ensures the changes commit to the file
  if (munmap(arr, FILESIZE) == -1) {
    printf("Error un-mmapping the file: %s\n", strerror(errno));
    return -1;
  }

  // un-mmaping doesn't close the file,
  //  so we still need to do that.
  close(fd);

  // Look at the mapped file
  return 0;
}
