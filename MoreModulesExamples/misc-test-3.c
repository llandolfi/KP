#include <linux/poll.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/mutex.h>
#include <linux/slab.h>


MODULE_AUTHOR("Luca Abeni");
MODULE_DESCRIPTION("Test for misc devices");
MODULE_LICENSE("GPL");

static struct miscdevice test_device;
static char *my_pointer;
static int my_len;
static struct mutex my_mutex;

static int my_open(struct inode *inode, struct file *file)
{
  return 0;
}

static int my_close(struct inode *inode, struct file *file)
{
  return 0;
}

ssize_t my_read(struct file *file, char __user *buf, size_t len, loff_t *ppos)
{
  int err, res;

  mutex_lock(&my_mutex);
  if (len > my_len) {
    res = my_len;
  } else {
    res = len;
  }
  if (my_pointer == NULL) {
    mutex_unlock(&my_mutex);
    return 0;
  }
  err = copy_to_user(buf, my_pointer, res);
  if (err) {
    return -EFAULT;
  }
  kfree(my_pointer);
  my_pointer = NULL;
  mutex_unlock(&my_mutex);

  return res;
}

static ssize_t my_write(struct file *file, const char __user * buf, size_t count, loff_t *ppos)
{
  int err;

  mutex_lock(&my_mutex);
  if (my_pointer) {
    return -1;
  }
  my_pointer = kmalloc(count, GFP_USER);
  if (my_pointer == NULL) {
    return -1;
  }
  my_len = count;

  err = copy_from_user(my_pointer, buf, count);
  if (err) {
    return -EFAULT;
  }
  mutex_unlock(&my_mutex);

  return count;
}

static int testmodule_init(void)
{
  int res;

  res = misc_register(&test_device);

  printk("Misc Register returned %d\n", res);

  mutex_init(&my_mutex);

  return 0;
}

static void testmodule_exit(void)
{
  misc_deregister(&test_device);
}

static struct file_operations my_fops = {
  .owner =        THIS_MODULE,
  .read =         my_read,
  .open =         my_open,
  .release =      my_close,
  .write =        my_write,
#if 0
  .poll =         my_poll,
  .fasync =       my_fasync,
#endif
};

static struct miscdevice test_device = {
  MISC_DYNAMIC_MINOR, "test", &my_fops
};


module_init(testmodule_init);
module_exit(testmodule_exit);
