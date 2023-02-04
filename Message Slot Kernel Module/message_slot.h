
/*todo check about this indef, def*/
#ifndef OPERATING_SYSTEMS_MESSAGE_SLOT_H
#define OPERATING_SYSTEMS_MESSAGE_SLOT_H

#include <linux/ioctl.h>
#define MAJOR_NUM 235
#define MAX_BUF_LEN 128
#define MAX_NUM_OF_FILES 256
#define DEVICE_RANGE_NAME "message_slot"
#define SUCCESS 0

#define MSG_SLOT_CHANNEL _IOW(MAJOR_NUM, 0, unsigned long)
/*todo DEVICE_RANGE_NAME*/

typedef struct channel{
    unsigned int id;
    char* current_message; /*last message that was written to channel*/
    int message_length;
    struct channel* next; /*next channel*/
}channel;

typedef struct message_slot{
    channel* first_channel;
    channel* current_channel;
    int isSET; /*true iff message_slot has been set already (has channels)*/
}message_slot;

#endif
