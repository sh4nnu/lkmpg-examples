/*
 * creating a proc/ "file" that can be written and read.
 */ 

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h> /* for copy_from_user */

#define PROCFS_MAX_SIZE		1024
#define PROCFS_NAME		"buff1k"

static struct proc_dir_entry *Our_Proc_File;

static char procfs_buffer[PROCFS_MAX_SIZE]="HelloWorld!\n";


static unsigned long procfs_buffer_size = 0;

ssize_t procfile_read(struct file *filePointer, char *buffer,
			size_t buffer_length, loff_t * offset)
{
	int ret = 0;
	if(strlen(buffer) == 0) {
		pr_info("procfile read %s\n", filePointer->f_path.dentry->d_name.name);
		ret=copy_to_user(buffer,procfs_buffer,sizeof(procfs_buffer));
		ret=sizeof(procfs_buffer);
	}
	return ret;
}

static ssize_t procfile_write(struct file *file,const char *buff, size_t len, loff_t *off)
{
	procfs_buffer_size  = len;
	pr_info("procfile wrote %s\n", file->f_path.dentry->d_name.name);

	if(procfs_buffer_size > PROCFS_MAX_SIZE)
		procfs_buffer_size= PROCFS_MAX_SIZE;
	if (copy_from_user(procfs_buffer, buff, procfs_buffer_size))
        return -EFAULT;

    procfs_buffer[procfs_buffer_size] = '\0';
    return procfs_buffer_size;		
}


static const struct file_operations proc_file_fops = {
	.owner = THIS_MODULE,
	.read  = procfile_read,
	.write = procfile_write,
};


int init_module() 
{
	Our_Proc_File = proc_create(PROCFS_NAME, 0644,NULL, &proc_file_fops);
	if(Our_Proc_File == NULL) {
		proc_remove(Our_Proc_File);
		pr_alert("Error: Couldn't initialize /proc/%s\n", PROCFS_NAME);
		return -ENOMEM;	
	}
	
	pr_info("/proc/%s created\n", PROCFS_NAME);
	return 0;
}

void cleanup_module()
{
	proc_remove(Our_Proc_File);
	pr_info("/proc/%s removed \n", PROCFS_NAME);
}

