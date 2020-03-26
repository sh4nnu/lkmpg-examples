/*
 * end_module - the second file of multi sourced module
 */


#include <linux/kernel.h>
#include <linux/module.h>

void cleanup_module()
{
	pr_info(" End of multi sourced module\n");
}
