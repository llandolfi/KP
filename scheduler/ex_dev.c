#include <linux/poll.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/miscdevice.h>

#include "ex_thread.h"
#include "ex_dev.h"

static struct miscdevice thread_out_device;

static int thread_open(struct inode *inode, struct file *file)
{
  return 0;
  //TODO: maybe the open can add a thread to the thread list
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


int my_device_create(void)
{
  return misc_register(&thread_out_device);
}

void my_device_destroy(void)
{
  misc_deregister(&thread_out_device);
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
