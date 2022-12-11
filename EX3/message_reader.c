

#include "message_slot.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>

int main(int argc, char* argv[]){

    char* messageSlot_path;
    int fd;
    unsigned int channel_id;
    char message[MAX_BUF_LEN];
    int message_len;
    int ret_val;

    if (argc != 3){
        perror("ERROR : number of command line arguments is not valid");
        exit(1);
    }

    messageSlot_path = argv[1];
    channel_id = atoi(argv[2]);

    fd = open(messageSlot_path, O_RDWR);
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

    message_len = read(fd, message, MAX_BUF_LEN);
    if (message_len < 0){
        close(fd);
        perror("ERROR : read failed");
        exit(1);
    }
    close(fd);

    ret_val = write(STDOUT_FILENO,message, message_len);
    if (ret_val != message_len){
        perror("ERROR : write to STDOUT failed");
        exit(1);
    }
    exit(0);
}