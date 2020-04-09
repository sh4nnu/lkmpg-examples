/*
 *  This creates a sample proc/ file that returns "Hello World!" when tried to read.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>

#define procfs_name "helloworld" // name of the proc file.
struct proc_dir_entry *Our_Proc_File; // used to configure the procfile.

ssize_t procfile_read(struct file *filePointer, char *buffer, 
			size_t buffer_length, loff_t * offset)
{
	int ret=0;
	if(strlen(buffer) ==0) {
		pr_info("procfile read %s\n", filePointer->f_path.dentry->d_name.name);
		ret=copy_to_user(buffer,"HelloWorld!\n",sizeof("HelloWorld!\n"));
		ret=sizeof("HelloWorld!\n");
	}
	return ret;
}

static const struct file_operations proc_file_fops = {
	.owner = THIS_MODULE,
	.read  = procfile_read,
};

int init_module()
{
	Our_Proc_File = proc_create(procfs_name, 0644, NULL, &proc_file_fops);
	if(Our_Proc_File==NULL) {
		proc_remove(Our_Proc_File);
		pr_alert("Error: Could not initialize /proc/%s\n", procfs_name);
		return -ENOMEM;
	}
	
	pr_info("/proc/%s created\n", procfs_name);
	return 0;
}


void cleanup_module()
{
	proc_remove(Our_Proc_File);
	pr_info("/proc/%s removed\n", procfs_name);
}

