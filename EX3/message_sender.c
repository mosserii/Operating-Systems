
#include "message_slot.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>

int main(int argc, char* argv[]){
    char* slot_path;
    int fd;
    unsigned int channel_id;
    char* message;
    int ret_val;

    if (argc != 4){
        perror("ERROR : number of command line arguments is not valid");
        exit(1);
    }

    slot_path = argv[1];
    channel_id = atoi(argv[2]);
    message = argv[3];

    fd = open(slot_path,O_RDWR);
    if (fd < 0){
        perror("ERROR : file opening failed");
        exit(1);
    }

    ret_val = ioctl(fd, MSG_SLOT_CHANNEL, channel_id);
    if (ret_val != SUCCESS){
        close(fd);
        perror("ERROR : ioctl failed");
        exit(1);
    }

    ret_val = write( fd, message, strlen(message));
    if (ret_val < 0){
        close(fd);
        perror("ERROR : write failed");
        exit(1);
    }
    close(fd);
    exit(0);
}





