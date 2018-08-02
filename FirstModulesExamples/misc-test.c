#include <linux/poll.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>


MODULE_AUTHOR("Luca Abeni");
MODULE_DESCRIPTION("Test for misc devices");
MODULE_LICENSE("GPL");

static int my_open(struct inode *inode, struct file *file)
{
  int *count;

  count = kmalloc(sizeof(int), GFP_USER);
  if (count == NULL) {
    return -1;
  }

  *count = 0;
  file->private_data = count;
  
  printk("Device open!\n");

  return 0;
}

static int my_close(struct inode *inode, struct file *file)
{
  kfree(file->private_data);
  printk("Device close!\n");

  return 0;
}

ssize_t my_read(struct file *file, char __user *buf, size_t len, loff_t *ppos)
{
  int res, err, *count;

  count = file->private_data;
  *count = *count + 1;
  if (*count == 10) {
    return 0;
  }

  if (len > 10) {
    res = 10;
  } else {
    res = len;
  }
  err = copy_to_user(buf, "hi there!\n", res);
  if (err) {
    return -EFAULT;
  }
  printk("Read %ld (%d)\n", len, *count);

  return res;
}

static struct file_operations my_fops = {
  .owner =        THIS_MODULE,
  .read =         my_read,
  .open =         my_open,
  .release =      my_close,
#if 0
  .write =        my_write,
  .poll =         my_poll,
  .fasync =       my_fasync,
#endif
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
