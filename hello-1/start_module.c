/*
 * start_module.c - illustration of modules with multiple source files
 */ 

#include <linux/kernel.h>
#include <linux/module.h>

int init_module(void) 
{
	pr_info("first part of multi source module\n");
	return 0;
}
