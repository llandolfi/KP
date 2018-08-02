#include <linux/poll.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>


MODULE_AUTHOR("Luca Abeni");
MODULE_DESCRIPTION("Test for misc devices");
MODULE_LICENSE("GPL");

static char v;

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
  int res, err;
  char vv[2];

  if (v == 0) {
    return 0;
  }
  if (len > 2) {
    res = 2;
  } else {
    res = len;
  }
  vv[0] = vv[1] = v;

  err = copy_to_user(buf, vv, res);
  if (err) {
    return -EFAULT;
  }

  v = 0;

  return res;
}

static ssize_t my_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
  int err;

  err = copy_from_user(&v, buf, 1);
  if (err) {
    return -EFAULT;
  }

  return count;
}

static struct file_operations my_fops = {
  .owner =        THIS_MODULE,
  .read =         my_read,
  .open =         my_open,
  .release =      my_close,
  .write =        my_write,
};

static struct miscdevice test_device = {
  MISC_DYNAMIC_MINOR, "test", &my_fops
};


static int testmodule_init(void)
{
  int res;

  res = misc_register(&test_device);

  printk("Misc Register returned %d\n", res);

  return 0;
}

static void testmodule_exit(void)
{
  misc_deregister(&test_device);
}

module_init(testmodule_init);
module_exit(testmodule_exit);
