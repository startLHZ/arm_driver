// test_rcu_stall.c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/delay.h>

static int choice;
module_param(choice, int, 0660);

MODULE_PARM_DESC(choice, "Choice of test: 0 for other CPU, 1 for current CPU");

static int __init test_init(void)
{
    if (choice == 0) {
        /* 测试：直接调用 stall 函数 */
        extern void print_other_cpu_stall(unsigned long gps);
        printk("RCU Monitor: Testing - calling print_other_cpu_stall directly\n");
        print_other_cpu_stall(jiffies);
    } else {
        /* 测试：直接调用 stall 函数 */
        extern void print_cpu_stall(unsigned long gps);
        printk("RCU Monitor: Testing - calling print_cpu_stall directly\n");
        print_cpu_stall(jiffies);
    }
    
    return 0;
}

static void __exit test_exit(void)
{
    printk("RCU stall test module unloaded\n");
}

module_init(test_init);
module_exit(test_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Test");
MODULE_DESCRIPTION("RCU Stall Test Module");