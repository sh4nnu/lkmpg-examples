/*
 *  chardev.c - Create an input/output character device
 */

#include <linux/kernel.h>       
#include <linux/module.h>       
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/irq.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <linux/poll.h>
#include <linux/cdev.h>

#include "chardev.h"
#define SUCCESS 0
#define DEVICE_NAME "char_dev"
#define BUF_LEN 80

MODULE_LICENSE("GPL");

/*
 * status of device whether it's open.
 * used to avoid concurrent access to file(device).
 */
static int Device_Open = 0;

/*
 * The message buffer.
 */
static char Message[BUF_LEN];

/*
 * pointer to track till where the device_read read, 
 * useful incase of larger messages.
 */
static char *Message_Ptr;

static int Major;               /* Major number assigned to our device driver */
static struct class *cls;

/*
 * This is called whenever a process attempts to open the device file
 */
static int device_open(struct inode *inode, struct file *file)
{
#ifdef DEBUG
        pr_info("device_open(%p)\n", file);
#endif

    /*
     * Communicate with one process at a time.
     */
    if (Device_Open)
        return -EBUSY;

    Device_Open++;

    Message_Ptr = Message;
    try_module_get(THIS_MODULE);
    return SUCCESS;
}

static int device_release(struct inode *inode, struct file *file)
{
#ifdef DEBUG
    pr_info("device_release(%p,%p)\n", inode, file);
#endif

    Device_Open--;

    module_put(THIS_MODULE);
    return SUCCESS;
}


static ssize_t device_read(struct file *file,  
                           char __user * buffer,       
                           size_t length,
                           loff_t * offset)
{
    int bytes_read = 0;

#ifdef DEBUG
    pr_info("device_read(%p,%p,%d)\n", file, buffer, length);
#endif

    /*
     * If at the end of the message, return 0
     */
    if (*Message_Ptr == 0)
        return 0;

    /*
     * Filling the Buffer
     */
    while (length && *Message_Ptr) {

    /*
     * Copy data from kernel data segment to user data segment.
     */
     put_user(*(Message_Ptr++), buffer++);
     length--;
     bytes_read++;
}

#ifdef DEBUG
    pr_info("Read %d bytes, %d left\n", bytes_read, length);
#endif

    return bytes_read;
}


static ssize_t
device_write(struct file *file,
             const char __user * buffer, size_t length, loff_t * offset)
{
    int i;

#ifdef DEBUG
    pr_info("device_write(%p,%s,%d)", file, buffer, length);
#endif

    for (i = 0; i < length && i < BUF_LEN; i++)
        get_user(Message[i], buffer + i);

    Message_Ptr = Message;
    return i;
}

/*
 * This function is called whenever a function does ioctl functionality.
 * If the ioctl is write or read/write (meaning output is returned to the
 * calling process), the ioctl call returns the output of this function.
 *
 */
long device_ioctl(struct file *file,
                  unsigned int ioctl_num,        
                  unsigned long ioctl_param)
{
    int i;
    char *temp;
    char ch;

    /*
     * appropriate operations according to ioctl_num
     */
    switch (ioctl_num) {
    case IOCTL_SET_MSG:
        /*
         * Receive a pointer to a message (in user space) and set that
         * to be the device's message.
         */
        temp = (char *)ioctl_param;

         get_user(ch, temp);
         for (i = 0; ch && i < BUF_LEN; i++, temp++)
             get_user(ch, temp);

         device_write(file, (char *)ioctl_param, i, 0);
         break;

    case IOCTL_GET_MSG:
        /*
         * Give the current message to the calling process -
         * the parameter we got is a pointer, fill it.
         */
        i = device_read(file, (char *)ioctl_param, 99, 0);

        /*
         * terminate the data in the buffer.
         */
        put_user('\0', (char *)ioctl_param + i);
        break;

    case IOCTL_GET_NTH_BYTE:
        /*
         * This ioctl is both input (ioctl_param) and
         * output (the return value of this function)
         */
        return Message[ioctl_param];
        break;
    }

    return SUCCESS;
}

/* Module Declarations */

struct file_operations Fops = {
        .read = device_read,
        .write = device_write,
        .unlocked_ioctl = device_ioctl,
        .open = device_open,
        .release = device_release,     
};

int init_module()
{
    int ret_val;
    ret_val = register_chrdev(MAJOR_NUM, DEVICE_FILE_NAME, &Fops);
    pr_info("%d this major num %d", ret_val, MAJOR_NUM);
    if (ret_val < 0) {
        pr_alert("%s failed with %d\n",
                 "Sorry, registering the character device ", ret_val);
        return ret_val;
    }

    Major = ret_val;

    cls = class_create(THIS_MODULE, DEVICE_FILE_NAME);
    device_create(cls, NULL, MKDEV(MAJOR_NUM, 0), NULL, DEVICE_FILE_NAME);
    pr_info("Major number assigned %d\n", Major);
    pr_info("Device created on /dev/%s\n", DEVICE_FILE_NAME);

    return 0;
}


void cleanup_module()
{
    device_destroy(cls, MKDEV(Major, MAJOR_NUM));
    class_destroy(cls);
    unregister_chrdev(Major, DEVICE_FILE_NAME);
}