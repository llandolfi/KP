#include <linux/poll.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/completion.h>


MODULE_AUTHOR("Luca Abeni");
MODULE_DESCRIPTION("Test for misc devices");
MODULE_LICENSE("GPL");

static struct miscdevice test_device;
static char *my_pointer;
static int my_len;
static struct mutex my_mutex;
static struct completion my_completion;

static int my_open(struct inode *inode, struct file *file)
{
  int *count;

  count = kmalloc(sizeof(int), GFP_USER);
  if (count == NULL) {
    return -1;
  }

  *count = 0;
  file->private_data = count;
  
  return 0;
}

static int my_close(struct inode *inode, struct file *file)
{
  kfree(file->private_data);
  return 0;
}

ssize_t my_read(struct file *file, char __user *buf, size_t len, loff_t *ppos)
{
  int err, res;
  int *count;

  count = file->private_data;
  if (*count == 1) {
    return 0;
  }
  wait_for_completion(&my_completion);
  mutex_lock(&my_mutex);
  if (my_pointer == NULL) {
    printk("This should not happen...\n");
    BUG();
  }
  if (len > my_len) {
    res = my_len;
  } else {
    res = len;
  }
  err = copy_to_user(buf, my_pointer, res);
  if (err) {
    mutex_unlock(&my_mutex);
    return -EFAULT;
  }
  kfree(my_pointer);
  my_pointer = NULL;
  *count = 1;
  mutex_unlock(&my_mutex);

  return res;
}

static ssize_t my_write(struct file *file, const char __user * buf, size_t count, loff_t *ppos)
{
  int err;

  mutex_lock(&my_mutex);
  if (my_pointer) {
    mutex_unlock(&my_mutex);
    return -1;
  }
  my_pointer = kmalloc(count, GFP_USER);
  if (my_pointer == NULL) {
    mutex_unlock(&my_mutex);
    return -1;
  }
  my_len = count;

  err = copy_from_user(my_pointer, buf, count);
  if (err) {
    mutex_unlock(&my_mutex);
    return -EFAULT;
  }

  complete(&my_completion);
  mutex_unlock(&my_mutex);

  return count;
}

static int testmodule_init(void)
{
  int res;

  res = misc_register(&test_device);

  printk("Misc Register returned %d\n", res);

  mutex_init(&my_mutex);
  init_completion(&my_completion);

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
};

static struct miscdevice test_device = {
  MISC_DYNAMIC_MINOR, "test", &my_fops
};


module_init(testmodule_init);
module_exit(testmodule_exit);
