/*
 *  chardev.c: Creates a read-only char device that says how many times
 *  you've read from the dev file. In this case `/dev/chardev`
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
MODULE_LICENSE("GPL");

//prototypes

int init_module(void);
void cleanup_module(void);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

// Macros

#define SUCCESS 0
#define DEVICE_NAME "chardev"   /* Dev name as it appears in /proc/devices   */
#define BUF_LEN 80              /* Max length of the message from the device */

static int Major;               /* Major number assigned to our device driver */
static int Device_Open = 0;     /* Is device open?
                                 * Used to prevent multiple access to device */
static char msg[BUF_LEN];       /* The msg the device will give when asked */
static char *msg_Ptr;

static struct class *cls; 


static struct file_operations chardev_fops = {
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release
};

int init_module(void)
{
	Major = register_chrdev(0, DEVICE_NAME, &chardev_fops);
	
	if(Major < 0) {
		pr_alert("Registering the char device is failed with %d\n", Major);
		return Major;
	}

	pr_info(" The Major number assigned : %d.\n", Major);

	cls = class_create(THIS_MODULE, DEVICE_NAME);
	device_create(cls, NULL, MKDEV(Major, 0), NULL, DEVICE_NAME);

	pr_info("DEvice created on /dev/%s\n", DEVICE_NAME);
	
	return SUCCESS;
}
void cleanup_module(void)
{
    device_destroy(cls, MKDEV(Major, 0));
    class_destroy(cls);

    /*
     * Unregister the device
     */
    unregister_chrdev(Major, DEVICE_NAME);
}

/*
 * Methods
 */

/* 
 * Called when a process tries to open the device file, like
 * `cat /dev/some_file`
 */

static int device_open(struct inode *inode, struct file *file)
{
    static int counter = 0;

    if (Device_Open)
        return -EBUSY;

    Device_Open++;
    sprintf(msg, "I already told you %d times Hello world!\n", counter++);
    msg_Ptr = msg;
    try_module_get(THIS_MODULE);

    return SUCCESS;
}


/*
 * Called when a process closes the device file.
 */

static int device_release(struct inode *inode, struct file *file)
{
	Device_Open--;

	module_put(THIS_MODULE);
	
	return SUCCESS;
}


/*
 * Called when a process attempts to read from a file that was already opened 
 * by that process.
 */

static ssize_t device_read(struct file *filp,
			   char *buffer,
			   size_t length, 
			   loff_t * offset)
{
	int bytes_read = 0;
	if(*msg_Ptr == 0)
		return 0;
	
	while( length && *msg_Ptr ) {
		put_user(*(msg_Ptr++), buffer++);
		length--;
		bytes_read++;
	}
	
	return bytes_read++;
}
static ssize_t device_write(struct file *flip,
			    const char *buff,
			    size_t len,
			    loff_t * off)
{

	pr_alert("Sorry, this is WIP");
	return -EINVAL;
}
