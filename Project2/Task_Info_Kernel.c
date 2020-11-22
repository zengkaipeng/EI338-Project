#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/jiffies.h>
#include <linux/sched.h>

#define BUFFER_SIZE 256
#define PROC_NAME "pid"

static long pid_num = 0;

ssize_t proc_read(
	struct file *file, char __user *usr_buf,
	size_t count, loff_t *pos
);

ssize_t proc_write(
	struct file *file, const char __user *usr_buf,
	size_t count, loff_t *pos
);

static struct file_operations proc_ops = {
	.owner = THIS_MODULE,
	.read = proc_read,
	.write = proc_write,
};

int proc_init(void)
{
	/* creates the /proc/hello entry */
	proc_create(PROC_NAME, 0666, NULL, &proc_ops);
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
	struct pid *Vpid = NULL;
	struct task_struct *tsk = NULL;
	if(completed)
	{
		completed = 0;
		return 0;
	}
	// printk(KERN_INFO "pnum: %ld", pid_num);
	completed = 1;
	Vpid = find_vpid(pid_num);
	tsk = pid_task(Vpid, PIDTYPE_PID);
	if(tsk == NULL) return 0;
	
	rv = sprintf(
		buffer, "command = [%s] pid=[%d] state = [%ld]\n",
		tsk -> comm, tsk -> pid, tsk -> state
	);
	
	copy_to_user(usr_buf, buffer, rv);
	return rv;
}

ssize_t proc_write(
	struct file *file, const char __user *usr_buf,
	size_t count, loff_t *pos
)
{
	char *k_mem;
	k_mem = kmalloc(count, GFP_KERNEL);
	copy_from_user(k_mem, usr_buf, count);
	kstrtol(k_mem, 10, &pid_num);
	// printk(KERN_INFO "num: %ld\n", pid_num);
	kfree(k_mem);
	return count;
}

module_init(proc_init);
module_exit(proc_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Task Information Kernel");
MODULE_AUTHOR("SCG");

