#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/delay.h>

MODULE_AUTHOR("Luca Abeni");
MODULE_DESCRIPTION("Test for kernel threads");
MODULE_LICENSE("GPL");

static struct task_struct *my_thread_descr;

static int my_kernel_thread(void *arg)
{
  int i, cnt = 0;

  while (!kthread_should_stop()) {
    printk("Activation %d\n", cnt++);
    set_current_state(TASK_INTERRUPTIBLE);
    schedule_timeout(1 * HZ);
  }

  printk("Stopping");
  for (i = 0; i < 5; i++) {
    set_current_state(TASK_INTERRUPTIBLE);
    schedule_timeout(HZ);
    printk(".");
  }
  printk("\n");

  return 0;
}

static int __init my_init(void)
{
  my_thread_descr = kthread_run(my_kernel_thread, NULL, "test_thread");
  if (IS_ERR(my_thread_descr)) {
    printk("Error creating kernel thread!\n");

    return PTR_ERR(my_thread_descr);
  }

  return 0;
}

static void __exit my_exit(void)
{
  int res;
  
  res = kthread_stop(my_thread_descr);

  printk("Killed: %d\n", res);
}

module_init(my_init);
module_exit(my_exit);
