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
#include <linux/errno.h>
#include <linux/slab.h>


MODULE_LICENSE("GPL");

typedef struct channel{
    unsigned int id;
    char* current_message;
    int message_length;
    struct channel* next;
}channel;

typedef struct message_slot{
    channel* first_channel;
    channel* current_channel;
    int isSET;
}message_slot;

/*data structure to describe individual message slots*/
static message_slot* message_slots_array[MAX_NUM_OF_FILES + 1];/*todo check if +1*/



//================== DEVICE FUNCTIONS ===========================
static int device_open( struct inode* inode,
                        struct file*  file ){

    int minor;
    channel* channel1;
    message_slot* messageSlot;

    printk(KERN_ALERT "device_open invoked\n");

    minor = iminor(inode);
    printk(KERN_DEBUG "minor = %d\n", minor);
    if (message_slots_array[minor] == NULL){
        printk("(message_slots_array[minor] == NULL) in device_open\n");
        messageSlot = (message_slot*) kmalloc(sizeof(message_slot), GFP_KERNEL);
        channel1 = (channel *) kmalloc(sizeof(channel), GFP_KERNEL);
        if (messageSlot == NULL || channel1 == NULL){
            printk("kmalloc failed in device_open\n");
            return -1;
            /*todo return  einmem*/
        }

        messageSlot->first_channel = channel1;
        messageSlot->isSET = 0;/*todo check if 0 or 1 needed*/
        message_slots_array[minor] = messageSlot;

    }
    printk("device_open succeeded\n");
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

    printk(KERN_ALERT "device_read invoked\n");

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
    printk(KERN_DEBUG "messageSlot = %p\n", messageSlot);

    channel1 = messageSlot->current_channel;
    if (channel1 == NULL){
        printk("channel1 is NULL in device_read\n");
        return -EINVAL;
    }
    printk(KERN_DEBUG "channel1 = %p\n", channel1);


    /*todo check here!!!!!!!!!!, message_len == 0*/
    message = channel1->current_message;
    message_len = channel1->message_length;
    if (message == NULL){
        printk("(message == NULL) in device_read\n");
        return -EWOULDBLOCK;
    }
    printk(KERN_DEBUG "message = %p, channel1->current_message = %p, message_len = %d\n", message, channel1->current_message, message_len);

    if(length < message_len){
        printk("buffer length is smaller than last message length");
        return -ENOSPC;
    }

    for(i = 0; i < message_len; i++){
        if((put_user(message[i], &buffer[i])) != 0)
            return -1;/*todo maybe different error*/
    }
    printk("%d bytes were read from device\n", i);
    return i;
}

//---------------------------------------------------------------
// a processs which has already opened
// the device file attempts to write to it
static ssize_t device_write( struct file*       file,
                             const char __user* buffer,
                             size_t             length,
                             loff_t*            offset){

    char* message;
    channel* channel1;
    message_slot* messageSlot;
    int i;

    printk("device_write invoked\n");

    if ( file == NULL ){
        printk("file is NULL\n");
        return -EINVAL;/*todo check*/
    }

    if (length > MAX_BUF_LEN || length == 0){
        printk("illegal length in device_write() \n");
        return -EMSGSIZE;
    }

    messageSlot = (message_slot *) (file->private_data);
    if (messageSlot == NULL){
        printk("messageSlot is NULL\n");
        return -EINVAL;
    }
    printk("messageSlot = %p\n", messageSlot);


    channel1 = messageSlot->current_channel;
    if (channel1 == NULL){
        printk("channel1 is NULL\n");
        return -EINVAL;
    }
    printk("channel1 = %p\n", channel1);


    /*not an error, we just allocate space for message inside the channel*/
    if (channel1->current_message == NULL){
        printk("not an error, channel1 message is NULL\n");
        channel1 -> current_message = (char *) kmalloc(MAX_BUF_LEN, GFP_KERNEL);
        if(channel1->current_message == NULL){ /*kmalloc failed!*/
            printk("kmalloc failed in allocating message space inside channel\n");
            return -1;
        }
    }

    /*message = channel1->current_message;*/
    /*todo check here, maybe lentgh instead of MAX_BUF_LEN*/
    memset(channel1->current_message, 0, sizeof(length));

    channel1->message_length = length;
    for(i = 0; i < length; i++){
        if(get_user(channel1->current_message[i], &buffer[i]) != 0){
            printk("i = %d get_user(message[i], &buffer[i]) != 0\n", i);
            return -1;
        }
    }
    /*todo check if i == length because its atomic?*/
    /* return the number of written bytes*/
    printk("%d bytes were written to device\n", i);
    return i;
}

//----------------------------------------------------------------
static long device_ioctl( struct   file* file,
                          unsigned int   ioctl_command_id,
                          unsigned long  ioctl_param ){

    message_slot* messageSlot;
    channel* channel_ptr;
    channel* new_channel;
    channel* prev_channel;
    int already_exists = 0;
    int minor;
    prev_channel = NULL; /*in case we do not enter the if expression*/
    printk("device_ioctl invoked\n");
    printk("ioctl_param = %lu\n", ioctl_param);



    if(MSG_SLOT_CHANNEL != ioctl_command_id || 0 == ioctl_param) {
        printk("(MSG_SLOT_CHANNEL != ioctl_command_id || 0 == ioctl_param)\n");
        return -EINVAL;
    }

    if(file->f_inode == NULL) {
        printk("(file->f_inode == NULL)\n");
        return -EINVAL;
    }
    minor = iminor(file->f_inode);

    if(message_slots_array[minor]==NULL){
        printk("message_slot_array and file error\n");
        return -1;/*todo check*/
    }

    file->private_data = (void*) message_slots_array[minor];/*todo check!!!!!*/


    messageSlot = (message_slot*) (file->private_data);
    if (messageSlot == NULL){
        printk("messageSlot is NULL in ioctl()\n");
        return -EINVAL;
    }
    printk("messageSlot in ioctl = %p\n", messageSlot);

    channel_ptr = (channel*) (messageSlot->first_channel);
    printk("first_channel_ptr oof messageSlot in ioctl = %p\n", channel_ptr);

    if (messageSlot->isSET) { /*we have already initialized this slot and it has at least one channel*/
        printk("(messageSlot isSET)\n");
        while (channel_ptr != NULL) {/*todo big check here*/
            printk("(while (channel_ptr != NULL))\n");
            printk("channel_ptr id k = %u\n", channel_ptr->id);
            if (channel_ptr->id == ioctl_param) {
                printk("channel is already exists\n");
                already_exists = 1;
                messageSlot->current_channel = channel_ptr;
                break;
            }
            /*keep going till you find the channel is already exists or channel_ptr is NULL*/
            prev_channel = channel_ptr;
            channel_ptr = channel_ptr->next;
        }
    }

    /*channel creation*/
    if (!already_exists || !(messageSlot->isSET)){
        /*todo check if channel_ptr is the right variable to mess with*/
        new_channel = (channel*) kmalloc(sizeof(channel), GFP_KERNEL);
        if (new_channel == NULL){
            printk("new_channel kmalloc failed\n");
            return -1;
        }
        new_channel->id = ioctl_param;
        new_channel->next = NULL;
        new_channel->current_message = NULL;
        new_channel->message_length = 0;
        if (!messageSlot->isSET){
            printk("first channel in this file\n");
            messageSlot->first_channel->next = new_channel;/*todo check be careful*/
            messageSlot->isSET = 1;
        }
        else {/*messageSlot is already set*/
            prev_channel->next = new_channel;/*adding new_channel to the end of the linked list*/
        }
        messageSlot->current_channel = new_channel;/*todo check, if you change the name so be careful when it was already exists*/
    }
    printk("messageSlot->first_channel at the end of ioctl = %p\n", messageSlot->first_channel);
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
               DEVICE_RANGE_NAME, MAJOR_NUM );/*todo check if it is DEVICE_FILE_NAME or file_name*/
        return rc;
    }
    printk( "Registeration is successful, major number is : %d \n", MAJOR_NUM);

    return SUCCESS;
}

//---------------------------------------------------------------
static void __exit simple_cleanup(void){ /*todo functions and cleaning here*/
    channel* channel_ptr;
    channel* next_channel;
    unregister_chrdev(MAJOR_NUM, DEVICE_RANGE_NAME);

    int j;
    for (j = 0; j < MAX_NUM_OF_FILES; j++) {
        if (message_slots_array[j] != NULL){
            /*cleaning message slot number j*/
            channel_ptr = message_slots_array[j]->first_channel;
            while(channel_ptr != NULL){
                next_channel = channel_ptr->next;
                if (channel_ptr->current_message != NULL)
                    kfree(channel_ptr->current_message);
                kfree(channel_ptr);
                channel_ptr = next_channel;
            }
            kfree(message_slots_array[j]);
        }
    }
}

//---------------------------------------------------------------
module_init(simple_init);
module_exit(simple_cleanup);

//========================= END OF FILE =========================