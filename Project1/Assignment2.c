#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/jiffies.h>
#include <asm/param.h>

#define BUFFER_SIZE 128
#define PROC_NAME "seconds"

const int hz = HZ;

unsigned long volatile load_jif, now_jif;

ssize_t proc_read(
	struct file *file, char __user *usr_buf,
	size_t count, loff_t *pos
);

static struct file_operations proc_ops = {
	.owner = THIS_MODULE,
	.read = proc_read,
};

int proc_init(void)
{
	/* creates the /proc/hello entry */
	proc_create(PROC_NAME, 0666, NULL, &proc_ops);
	load_jif = jiffies;
	return 0;
}

void proc_exit(void)
{
	remove_proc_entry(PROC_NAME, NULL);
}

ssize_t proc_read(
	struct file *file, char __user *usr_buf,
	size_t count, loff_t *pos
)
{
	int rv = 0;	
	char buffer[BUFFER_SIZE];
	static int completed = 0;
	if(completed)
	{
		completed = 0;
		return 0;
	}
	completed = 1;
	now_jif = jiffies;
	rv = sprintf(buffer, "%lu\n", (now_jif - load_jif) / hz);
	copy_to_user(usr_buf, buffer, rv);
	return rv;
}

module_init(proc_init);
module_exit(proc_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Assignment2");
MODULE_AUTHOR("SCG");

