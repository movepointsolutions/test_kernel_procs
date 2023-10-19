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

struct pid_list
{
    unsigned pid;
    struct list_head list;
};

struct tkp_private
{
    unsigned current_pid;
    int current_pid_set;
    struct list_head list;
    char buf[4096];
    char *cur;
    char *end;
};

static int tkp_open(struct inode *inode, struct file *file)
{
    struct tkp_private *private;
    printk(KERN_INFO "open\n");
    private = kzalloc(sizeof(struct tkp_private), GFP_KERNEL);
    INIT_LIST_HEAD(&private->list);
    private->cur = private->end = private->buf;
    file->private_data = private;
    return 0;
}

static int tkp_close(struct inode *inode, struct file *file)
{
    struct tkp_private *private;
    printk(KERN_INFO "close\n");
    private = file->private_data;
    while (!list_empty(&private->list)) {
        struct pid_list *node = container_of(private->list.next, struct pid_list, list);
        printk(KERN_INFO "unread pid: %u\n", node->pid);
        list_del(&node->list);
        kfree(node);
    }
    kfree(private);
    return 0;
}

static void prepare_buffer(struct tkp_private *priv)
{
    ssize_t cnt;
    if (priv->cur == priv->end) {
        while (!list_empty(&priv->list)) {
            struct pid_list *node = container_of(priv->list.next, struct pid_list, list);
            struct task_struct *task = get_pid_task(find_get_pid(node->pid), PIDTYPE_PID);
            int ret = 0;
            if (task) {
                unsigned uid = task->cred->uid.val;
                char *pathname, *p;
                struct mm_struct *mm = task->mm;
                printk(KERN_INFO "read pid: %u\n", node->pid);
                if (mm) {
                    if (mm->exe_file) {
                        pathname = kmalloc(PATH_MAX, GFP_ATOMIC);
                        if (pathname) {
                            p = d_path(&mm->exe_file->f_path, pathname, PATH_MAX);
                            /*Now you have the path name of exe in p*/
                        }
                    }
                }
                cnt = snprintf(priv->buf, sizeof(priv->buf),
                        "PID: %u\nUID: %u\nPATH: %s\n", node->pid, uid, p);
                kfree(pathname);
                //printk(KERN_INFO "cnt: %lu\n", cnt);
                priv->cur = priv->buf;
                priv->end = priv->buf + cnt;
                ret = 1;
            }
            list_del(&node->list);
            kfree(node);
            if (ret)
                return;
        }
    }
}

static ssize_t tkp_read(struct file *file, char __user *buf,
        size_t count, loff_t *ppos)
{
    struct tkp_private *private;
    printk(KERN_INFO "read\n");
    private = file->private_data;
    prepare_buffer(private);
    //printk(KERN_INFO "read %p %p\n", private->cur, private->end);
    if (private->cur < private->end) {
        size_t cnt = private->end - private->cur;
        if (count < cnt)
            cnt = count;
        //printk(KERN_INFO "copy_to_user %li\n", cnt);
        if (copy_to_user(buf, private->buf, cnt))
            return -EFAULT;
        private->cur += cnt;
        //printk(KERN_INFO "return %li\n", cnt);
        return cnt;
    }
    return 0;
}


static ssize_t tkp_write(struct file *file, const char __user *buf,
        size_t count, loff_t *ppos)
{
    struct tkp_private *private;
    const char *i;
    printk(KERN_INFO "write\n");
    private = file->private_data;
    printk(KERN_INFO "current_pid_set %i\n", private->current_pid_set);
    for (i = buf; i < buf + count; i++) {
        char c;
        get_user(c, i);
        if (c >= '0' && c <= '9') {
            private->current_pid_set = 1;
            private->current_pid = private->current_pid * 10 + (c - '0');
        } else {
            if (private->current_pid_set) {
                struct pid_list *node = kzalloc(sizeof(struct pid_list), GFP_KERNEL);
                node->pid = private->current_pid;
                private->current_pid = 0;
                list_add_tail(&node->list, &private->list);
            }
            private->current_pid_set = private->current_pid = 0;
        }
    }
    return count;
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
    unregister_chrdev(MAJOR_NUM, DEVICE_NAME);
    printk(KERN_INFO "Goodbye\n");
}

module_init(tkp_start);
module_exit(tkp_end);

// vim: autoindent expandtab tabstop=4 softtabstop=4 shiftwidth=4
