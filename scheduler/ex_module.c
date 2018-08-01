#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

#include "ex_thread.h"
#include "ex_dev.h"

MODULE_AUTHOR("Luca Abeni");
MODULE_DESCRIPTION("Example with devices and kthreads");
MODULE_LICENSE("GPL");

module_param(thread_num, int, 0);

static int __init scheduler_init(void)
{
  int res;
  int i;


  res = scheduler_create(thread_num, 1000);
  if (res < 0)
  {
    return res;
  }

 for (i=0; i < thread_num; i++)
  {
    res = thread_create(i);
    if (res < 0) {
      return res;
    }
  }

  res = my_device_create();

  if (res < 0) {

    scheduler_destroy();

    for (i=0; i < thread_num; i++)
    {
      thread_destroy(i);
    }

    return res;
  }
  
  return 0;
}

static void scheduler_cleanup(void)
{
  int i;
  scheduler_destroy();

  for (i=0; i < thread_num; i++)
  {
    thread_destroy(i);
  }
  my_device_destroy();
}

module_init(scheduler_init);
module_exit(scheduler_cleanup);

