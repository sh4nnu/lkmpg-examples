/*
 * procfs3.c
 * procfs with permissions and with a proc/ file with 
 * inode structures instead of procfs.
 */ 

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/uaccess.h> /* for copy_from_user */

#define PROCFS_MAX_SIZE		2048
#define PROCFS_NAME		"buff2k"
MODULE_LICENSE("GPL");

static struct proc_dir_entry *Our_Proc_File;

static char procfs_buffer[PROCFS_MAX_SIZE]="HelloWorld!\n";


static unsigned long procfs_buffer_size = 0;

static ssize_t procfile_read(struct file *filePointer, char *buffer,
			size_t buffer_length, loff_t * offset)
{
	static int finished = 0;
    if(finished)
    {
        pr_debug("procfs_read: END\n");
        finished = 0;
        return 0;
    }
    finished = 1;
    if(copy_to_user(buffer, procfs_buffer, procfs_buffer_size))
        return -EFAULT;
    pr_debug("procfs_read: read %lu bytes\n", procfs_buffer_size);
    return procfs_buffer_size;
}

static ssize_t procfile_write(struct file *file,const char *buff, size_t len, loff_t *off)
{
	if(len>PROCFS_MAX_SIZE)
        procfs_buffer_size = PROCFS_MAX_SIZE;
    else
        procfs_buffer_size = len;
    if(copy_from_user(procfs_buffer, buff,procfs_buffer_size))
        return -EFAULT;
    // pr_debug will work only when, CONFIG_DYNAMIC_DEBUG=y is set 
    // while building the kernel.

    pr_debug("procfs_write: write %lu bytes\n", procfs_buffer_size);
    return procfs_buffer_size;
}

int procfile_open(struct inode *inode, struct file *file)
{
    try_module_get(THIS_MODULE);
    return 0;
}

int procfile_close(struct inode *inode, struct file *file)
{
    module_put(THIS_MODULE);
    return 0;
}

static const struct file_operations proc_file_fops = {
    .open    = procfile_open,
    .release = procfile_close,
	.read    = procfile_read,
	.write   = procfile_write,
};


int init_module() 
{
	Our_Proc_File = proc_create(PROCFS_NAME, 0644,NULL, &proc_file_fops);
	if(Our_Proc_File == NULL) {
		proc_remove(Our_Proc_File);
		pr_alert("Error: Couldn't initialize /proc/%s\n", PROCFS_NAME);
		return -ENOMEM;
    }
    proc_set_size(Our_Proc_File, 80);
    proc_set_user(Our_Proc_File,  GLOBAL_ROOT_UID, GLOBAL_ROOT_GID);
    
	
	pr_info("/proc/%s created\n", PROCFS_NAME);
	return 0;
}

void cleanup_module()
{
	remove_proc_entry(PROCFS_NAME, NULL);
	pr_info("/proc/%s removed \n", PROCFS_NAME);
}

