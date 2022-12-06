#undef __KERNEL__
#define __KERNEL__
#undef MODULE
#define MODULE

#include "message_slot.h"
#include <linux/kernel.h>   /* We're doing kernel work */
#include <linux/module.h>   /* Specifically, a module */
#include <linux/fs.h>       /* for register_chrdev */
#include <linux/uaccess.h>  /* for get_user and put_user */
#include <linux/string.h>   /* for memset. NOTE - not string.h!*/



#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/errno.h>
#include <string.h>
#include <sys/fcntl.h>
#include <signal.h>









MODULE_LICENSE("GPL");

typedef struct channel{
    unsigned int id;
    char* current_message;
    int message_length;
}channel;

typedef struct message_slot{
    channel* first_channel;
    channel* current_channel;
    int isSET;
    int minor_num;
}message_slot;

/*data structure to describe individual message slots*/
static message_slot* message_slots_array[MAX_NUM_OF_FILES + 1];



//================== DEVICE FUNCTIONS ===========================
static int device_open( struct inode* inode,
                        struct file*  file ){

    int minor;
    minor = iminor(inode);
    channel* channel1;
    message_slot* messageSlot;

    printk("device_open invoked\n");

    if (message_slots_array[minor] == NULL){
        messageSlot = (message_slot*) kmalloc(sizeof(message_slot), GFP_KERNEL);
        channel1 = (channel *) kmalloc(sizeof(channel), GFP_KERNEL);
        if (messageSlot == NULL || channel1 == NULL){
            printk("kmalloc failed in device_open\n");
            return -1;
            /*todo return  einmem*/
        }


        messageSlot->first_channel = channel1;
        messageSlot->isSET = 0;
        messageSlot->minor_num = minor;

        message_slots_array[minor] = messageSlot;
    }
    return SUCCESS;
}





//---------------------------------------------------------------
static int device_release( struct inode* inode,
                           struct file*  file){
    printk("device_release invoked\n");
    kfree(file->private_data);
    return SUCCESS;
}

//---------------------------------------------------------------
// a process which has already opened
// the device file attempts to read from it
static ssize_t device_read( struct file* file,
                            char __user* buffer,
                            size_t       length,
                            loff_t*      offset){

    char* message;
    int message_len;
    channel* channel1;
    message_slot* messageSlot;
    int i;

    printk("device_read invoked\n");

    if ( file == NULL ){
    printk("file is NULL\n");
    return -EINVAL;
    }

    if (length > MAX_BUF_LEN || length == 0){
    printk("illegal length\n");
    return -EMSGSIZE;
    }

    messageSlot = (message_slot *) (file->private_data);
    if (messageSlot == NULL){
        printk("messageSlot is NULL\n");
        return -EINVAL;
    }
    channel1 = messageSlot->current_channel;
    if (channel1 == NULL){
    printk("channel1 is NULL\n");
    return -EINVAL;
    }
    message = channel1->current_message;
    message_len = channel1->message_length;
    if (message == NULL || message_len == 0){
        return -EWOULDBLOCK;
    }
    if(length < message_len){
        printk("buffer length is smaller than last message length");
        return -ENOSPC;
    }

    for(i = 0; i < message_len; i++){
        if(put_user(message[i], &buffer[i]) != 0)
            return -1;/*todo maybe different error*/
    }
    return i;
}

//---------------------------------------------------------------
// a processs which has already opened
// the device file attempts to write to it
static ssize_t device_write( struct file*       file,
                             const char __user* buffer,
                             size_t             length,
                             loff_t*            offset){

    channel* channel1;
    message_slot* messageSlot;
    int i;

    printk("device_write invoked\n");

    if ( file == NULL ){
        printk("file is NULL\n");
        return -EINVAL;/*todo check*/
    }

    if (length > MAX_BUF_LEN || length == 0){
        printk("illegal length\n");
        return -EMSGSIZE;
    }

    messageSlot = (message_slot *) (file->private_data);

    channel1 = messageSlot->current_channel;
    if (channel1 == NULL){
        printk("channel1 is NULL\n");
        return -EINVAL;
    }

    /*not an error, we just allocate space for message inside the channel*/
    if (channel1 -> current_message == NULL){
    printk("channel1 message is NULL\n");
    channel1 -> current_message = (char *) kmalloc(MAX_BUF_LEN, GFP_KERNEL);
    if(channel1 -> current_message == NULL) /*kmalloc failed!*/
        return -1;
    }

    channel1->message_length = length;
    for(i = 0; i < length; i++){
        if(get_user(channel1->current_message[i], &buffer[i]) != 0){
            return -1;
        }
    }
    /*todo check if i == length because its atomic?*/
    /* return the number of written bytes*/
    return i;
}

//----------------------------------------------------------------
static long device_ioctl( struct   file* file,
                          unsigned int   ioctl_command_id,
                          unsigned long  ioctl_param )
{
    // Switch according to the ioctl called
    if( IOCTL_SET_ENC == ioctl_command_id ) {
        // Get the parameter given to ioctl by the process
        printk( "Invoking ioctl: setting encryption "
                "flag to %ld\n", ioctl_param );
        encryption_flag = ioctl_param;
    }

    return SUCCESS;
}


//==================== DEVICE SETUP =============================
// This structure will hold the functions to be called
// when a process does something to the device we created
struct file_operations Fops = {
        .owner	  = THIS_MODULE,
        .read           = device_read,
        .write          = device_write,
        .open           = device_open,
        .unlocked_ioctl = device_ioctl,
        .release        = device_release,
};

//---------------------------------------------------------------
// Initialize the module - Register the character device
static int __init simple_init(void){
    int rc = -1;

    // Register driver capabilities. Obtain major num
    rc = register_chrdev(MAJOR_NUM, DEVICE_RANGE_NAME, &Fops);

    // Negative values signify an error
    if( rc < 0 ){
        printk("%s registraion failed for  %d\n",
                DEVICE_FILE_NAME, MAJOR_NUM );
        return rc;
    }
    printk( "Registeration is successful, major number is : %d \n", MAJOR_NUM);

    return 0;
}

//---------------------------------------------------------------
static void __exit simple_cleanup(void){ /*todo functions and cleaning here*/
    // Unregister the device
    // Should always succeed
    unregister_chrdev(MAJOR_NUM, DEVICE_RANGE_NAME);
}

//---------------------------------------------------------------
module_init(simple_init);
module_exit(simple_cleanup);

//========================= END OF FILE =========================