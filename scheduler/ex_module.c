#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/list.h>

#include "ex_thread.h"
#include "ex_dev.h"


MODULE_AUTHOR("Lorenzo Landolfi");
MODULE_DESCRIPTION("Simple Round Robin Scheduler");
MODULE_LICENSE("GPL");

int thread_num = 3;
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

static int __init scheduler_init_list(void)
{
  int res;
  int i;

  INIT_LIST_HEAD(&head);

  res = scheduler_create(thread_num, 1000);
  if (res < 0)
  {
    return res;
  }

 for (i=0; i < thread_num; i++)
  {
    res = thread_create_list(i);
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
  scheduler_destroy();

  printk("Scheduler destroyed \n");

  my_device_destroy();
  printk("Decvice destroyed\n");
}

static void scheduler_cleanup_list(void)
{
  scheduler_destroy();

  printk("Scheduler destroyed \n");

  my_device_destroy();
  printk("Decvice destroyed\n");
}

module_init(scheduler_init_list);
module_exit(scheduler_cleanup_list);

