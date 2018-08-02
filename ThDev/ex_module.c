#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

#include "ex_thread.h"
#include "ex_dev.h"

MODULE_AUTHOR("Luca Abeni");
MODULE_DESCRIPTION("Example with devices and kthreads");
MODULE_LICENSE("GPL");

static int __init example_init(void)
{
  int res;

  res = thread_create();
  if (res < 0) {
    return res;
  }

  res = my_device_create();
  if (res < 0) {
    thread_destroy();

    return res;
  }
  
  return 0;
}

static void example_cleanup(void)
{
  thread_destroy();
  my_device_destroy();
}

module_init(example_init);
module_exit(example_cleanup);

