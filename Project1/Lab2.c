#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/jiffies.h>
#include <asm/param.h>

int simple_init(void)
{
	// printk(KERN_INFO "Loading Kernel Module\n");
	printk(KERN_INFO "jiffies: %lu HZ: %d\n", jiffies, HZ);
	return 0;
}

void simple_exit(void)
{
	// printk(KERN_INFO "Removing Kernel Module\n");
	printk(KERN_INFO "jiffies: %lu\n", jiffies);
}

module_init(simple_init);
module_exit(simple_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("jiffies and HZ values");
MODULE_AUTHOR("SCG");