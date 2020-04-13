/*
 * seq_file.c
 * Create a "file" in procfs using seq_file API
 * 
 */ 

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#define PROCFS_NAME		"sequence_file"
MODULE_LICENSE("GPL");
/*
 * This function is called at the begining of the sequence
 * ie, when: 
 *    - the /proc file is read  (first time)
 *    - after the function stop (end of sequence)
 */
static void *seq_start(struct seq_file *s, loff_t *off) 
{
	static unsigned long counter = 0;
	
	// begining of a sequence
	if (*off == 0) {
		return &counter;
	}
	else {
	      // end of sequence return NULL.
		*off  = 0;
		return NULL;
	}
}

static void *seq_next(struct seq_file *s, void *v, loff_t *pos)
{
	unsigned long *tmp_v = (unsigned long*)v;
	(*tmp_v)++;
	(*pos)++;
	return NULL;
}

static void seq_stop(struct seq_file *s, void *v) 
{
	//Nothing to do.
}

static int seq_show(struct seq_file *s, void *v) 
{
	loff_t *spos = (loff_t *) v;

	seq_printf(s, "HELLOO %Ld\n", *spos);
	return 0;
}
// structure that holds the "functions" to manage the sequence.
static struct seq_operations seq_ops ={
	.start   = seq_start,
	.next    = seq_next,
	.stop    = seq_stop,
	.show    = seq_show,
};

// Called when the /proc file is opened.
static int file_open(struct inode *inode, struct file *file) 
{
	return seq_open(file, &seq_ops); 
}


static const struct file_operations proc_file_fops = {
    	.owner   = THIS_MODULE,
	.open    = file_open,
    	.read    = seq_read,
	.llseek  = seq_lseek,
	.release = seq_release,
};


int init_module() 
{
	struct proc_dir_entry *entry;
	
	entry = proc_create(PROCFS_NAME, 0, NULL, &proc_file_fops);
	if(entry == NULL)
	{
		remove_proc_entry(PROCFS_NAME, NULL);
		pr_debug("Error: Could not initialize /proc/%s \n", PROCFS_NAME);
		return -ENOMEM;
	} 
	return 0;
}

void cleanup_module()
{
	remove_proc_entry(PROCFS_NAME, NULL);
	pr_info("/proc/%s removed \n", PROCFS_NAME);
}

