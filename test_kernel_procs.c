/**
 * @file    tkp.c
 * @author  Akshat Sinha
 * @date    10 Sept 2016
 * @version 0.1
 * @brief  An introductory "Hello World!" loadable kernel
 *  module (LKM) that can display a message in the /var/log/kern.log
 *  file when the module is loaded and removed. The module can accept
 *  an argument when it is loaded -- the name, which appears in the
 *  kernel log files.
*/
#include <linux/module.h>     /* Needed by all modules */
#include <linux/kernel.h>     /* Needed for sscanf */
#include <linux/init.h>       /* Needed for the macros */
#include <linux/fs.h>       /* Needed for the char dev */
#include "test_kernel_procs.h"

///< The license type -- this affects runtime behavior
MODULE_LICENSE("GPL");

///< The author -- visible when you use modinfo
MODULE_AUTHOR("Anton Novikov and Akshat Sinha");

///< The description -- see modinfo
MODULE_DESCRIPTION("Kernel module to get running processes' info");

///< The version of the module
MODULE_VERSION("0.1");

static int tkp_open(struct inode *inode, struct file *file)
{
        return -1;
}

static int tkp_close(struct inode *inode, struct file *file)
{
        return -1;
}

static ssize_t tkp_read(struct file *file, char __user *buf,
                        size_t count, loff_t *ppos)
{
        return -1;
}


static ssize_t tkp_write(struct file *file, const char __user *buf,
                                      size_t count, loff_t *ppos)
{
        return -1;
}

static const struct file_operations tkp_fops = {
        .owner                = THIS_MODULE,
        .open                 = tkp_open,
        .release              = tkp_close,
        .read                 = tkp_read,
        .write                = tkp_write,
};

static int __init tkp_start(void)
{
    int ret;
    printk(KERN_INFO "Loading tkp module...\n");
    ret = register_chrdev(MAJOR_NUM, DEVICE_NAME, &tkp_fops);
    if (ret < 0) {
            printk ("%s failed with %d\n",
                    "Sorry, registering the character device ",
                    ret);
            return ret;
    }

    printk(KERN_INFO "Hello world\n");
    return 0;
}

static void __exit tkp_end(void)
{
    printk(KERN_INFO "Goodbye Mr.\n");
}

module_init(tkp_start);
module_exit(tkp_end);
