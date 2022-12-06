
/*todo check about this indef, def*/
#ifndef OPERATING_SYSTEMS_MESSAGE_SLOT_H
#define OPERATING_SYSTEMS_MESSAGE_SLOT_H

#define MAJOR_NUM 235
#define MAX_BUF_LEN 128
#define MAX_NUM_OF_FILES 256
#define DEVICE_RANGE_NAME "MESSAGE_SLOT"
#define SUCCESS 0

#define MSG_SLOT_CHANNEL _IOW(MAJOR_NUM, 0, unsigned long)
/*todo DEVICE_RANGE_NAME*/

#endif
