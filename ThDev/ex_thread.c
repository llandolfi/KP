#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/mutex.h>
#include <linux/completion.h>
#include <linux/delay.h>
#include <linux/uaccess.h>

#include "ex_thread.h"

static struct mutex buff_m;
struct completion available_data;

static char buff[1024];
static struct task_struct *out_id;

static int output_thread(void *arg)
{
  int cnt = 0;

  while (!kthread_should_stop()) {
    mutex_lock(&buff_m);
    sprintf(buff, "Activation %d\n", cnt++);
    mutex_unlock(&buff_m);
    complete(&available_data);
    msleep(500);
  }

  return 0;
}

int thread_create(void)
{
  mutex_init(&buff_m);
  init_completion(&available_data);
  
  out_id = kthread_run(output_thread, NULL, "out_thread");
  if (IS_ERR(out_id)) {
    printk("Error creating kernel thread!\n");

    return PTR_ERR(out_id);
  }

  return 0;
}

void thread_destroy(void)
{
  kthread_stop(out_id);
}

int get_data(char *p, int size)
{
  int len;

  mutex_lock(&buff_m);
  len = strlen(buff);
  if (len > size) {
    len = size;
  }
  if (copy_to_user(p, buff, len)) {
    len = -1;
  }
  mutex_unlock(&buff_m);

  return len;
}
