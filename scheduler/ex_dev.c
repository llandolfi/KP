#include <linux/poll.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/miscdevice.h>

#include "ex_thread.h"
#include "ex_dev.h"

static struct miscdevice thread_out_device;
static struct miscdevice append_device;
static struct miscdevice remove_device;

static int thread_open(struct inode *inode, struct file *file)
{
  return 0;
}

static int thread_close(struct inode *inode, struct file *file)
{
  return 0;
  //close can add a thread to the thread list scheduled by the RR scheduler
}

ssize_t thread_read(struct file *file, char __user *p, size_t len, loff_t *ppos)
{
  int res;
  wait_for_completion(&available_data);
  res = get_data(p, len);

  return res;
}

ssize_t dummy_read(struct file *file, char __user *p, size_t len, loff_t *ppos)
{
  return 0;
}

static int append_thread(struct inode *inode, struct file* file)
{
  int res;
  thread_num = thread_num + 1;
  res = thread_create_list(thread_num-1);
  
  return res;
}

static int remove_thread(struct inode *inode, struct file* file)
{
  return sched_rm_thread();
}


int my_device_create(void)
{ 
  int r1,r2,r3;
  r1 = misc_register(&thread_out_device);
  r2 = misc_register(&append_device);
  r3 = misc_register(&remove_device);

  return r1+r2+r3;
}

void my_device_destroy(void)
{
  misc_deregister(&thread_out_device);
  misc_deregister(&append_device);
  misc_deregister(&remove_device);
}

static struct file_operations thread_fops = {
  .owner =        THIS_MODULE,
  .read =         thread_read,
  .open =         thread_open,
  .release =      thread_close,
};

static struct miscdevice thread_out_device = {
  MISC_DYNAMIC_MINOR, "thout", &thread_fops
};


static struct file_operations append_fops = {
  .owner =        THIS_MODULE,
  .read =         dummy_read,
  .open =         append_thread,
  .release =      thread_close,
};


static struct miscdevice append_device = {
  MISC_DYNAMIC_MINOR, "thappend", &append_fops
};

static struct file_operations remove_fops = {
  .owner =        THIS_MODULE,
  .read =         dummy_read,
  .open =         remove_thread,
  .release =      thread_close,
};


static struct miscdevice remove_device = {
  MISC_DYNAMIC_MINOR, "thremove", &remove_fops
};