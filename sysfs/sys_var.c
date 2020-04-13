/*
 * sys_var.c sysfs example, where we create a variable in /sys that can be accessible 
 * from user space. Unlike /proc we can only implement show, store and OS will take care
 * of the inner implementations like open, read , write.
 */

#include <linux/module.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/string.h>

MODULE_LICENSE("GPL");

static struct kobject *module;

static int sysvar = 0;

static ssize_t variable_show(struct kobject *kobj,
                               struct kobj_attribute *attr,
                               char *buf)
{
    return sprintf(buf, "%d\n", sysvar);
}

static ssize_t variable_store(struct kobject *kobj,
                                struct kobj_attribute *attr,
                                char *buf, size_t count)
{
    sscanf(buf, "%du", &sysvar);
    return count;
}


static struct kobj_attribute variable_attribute =
    __ATTR(sysvar, 0660, variable_show,
           (void*)variable_store);

int init_module (void)
{
    int error = 0;

    pr_info("Initialised module\n");

    module =
        kobject_create_and_add("module", kernel_kobj);
    if (!module)
        return -ENOMEM;

    error = sysfs_create_file(module, &variable_attribute.attr);
    if (error) {
        pr_info("failed to create the variable file " \
               "in /sys/kernel/module\n");
    }

    return error;
}

void cleanup_module(void)
{
    pr_info("module: Exit success\n");
    kobject_put(module);
}
