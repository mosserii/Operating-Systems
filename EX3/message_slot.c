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

MODULE_LICENSE("GPL");

typedef struct channel{
    unsigned int id;
}channel;

typedef struct message_slot{
    channel* first_channel;
    channel* last_channel;
    int j;
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

    printk(KERN_ALERT "in device_open\n");

    if (message_slots_array[minor] == NULL){
        messageSlot = (message_slot*) kmalloc(sizeof(message_slot), GFP_KERNEL);
        channel1 = (channel *) kmalloc(sizeof(channel), GFP_KERNEL);
    }










    return SUCCESS;
}





//---------------------------------------------------------------
static int device_release( struct inode* inode,
                           struct file*  file)
{
    unsigned long flags; // for spinlock
    printk("Invoking device_release(%p,%p)\n", inode, file);

    // ready for our next caller
    spin_lock_irqsave(&device_info.lock, flags);
    --dev_open_flag;
    spin_unlock_irqrestore(&device_info.lock, flags);
    return SUCCESS;
}

//---------------------------------------------------------------
// a process which has already opened
// the device file attempts to read from it
static ssize_t device_read( struct file* file,
                            char __user* buffer,
size_t       length,
        loff_t*      offset )
{
// read doesnt really do anything (for now)
printk( "Invocing device_read(%p,%ld) - "
"operation not supported yet\n"
"(last written - %s)\n",
file, length, the_message );
//invalid argument error
return -EINVAL;
}

//---------------------------------------------------------------
// a processs which has already opened
// the device file attempts to write to it
static ssize_t device_write( struct file*       file,
                             const char __user* buffer,
size_t             length,
        loff_t*            offset)
{
ssize_t i;
printk("Invoking device_write(%p,%ld)\n", file, length);
for( i = 0; i < length && i < BUF_LEN; ++i ) {
get_user(the_message[i], &buffer[i]);
if( 1 == encryption_flag )
the_message[i] += 1;
}

// return the number of input characters used
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
        printk( KERN_ALERT "%s registraion failed for  %d\n",
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