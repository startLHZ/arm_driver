#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

static int __init simple_init(void)
{
    printk(KERN_INFO "Simple driver loaded\n");
    return 0;
}

static void __exit simple_exit(void)
{
    printk(KERN_INFO "Simple driver unloaded\n");
}

module_init(simple_init);
module_exit(simple_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Developer");
MODULE_DESCRIPTION("Simple ARM Driver");
MODULE_VERSION("1.0");
