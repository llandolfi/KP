#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/mutex.h>
#include <linux/completion.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/sched.h>

#include "ex_thread.h"

static struct mutex buff_m;
struct completion available_data;

static struct task_struct **out_id;
static struct task_struct *sched_id;

static struct mutex ready_mutex;

static wait_queue_head_t sched_waitqueue;
static wait_queue_head_t threads_waitqueue;

static int allready = 0;
static bool sched_ready = false;
static int turn = -1;

static char buff[1024];

int thread_num = 3;


static int output_thread(void *arg)
{

  int* my_id_p = (int*)arg;
  int my_id = *my_id_p;

  mutex_lock(&ready_mutex);
  allready = allready + 1;
  mutex_unlock(&ready_mutex);

  if (allready == thread_num)
  {
    wake_up_interruptible_all(&threads_waitqueue);
  }
  else
  {
    wait_event_interruptible(threads_waitqueue, (allready == thread_num));
  }

  wait_event_interruptible(sched_waitqueue, (sched_ready == true));

  while (!kthread_should_stop()) {
    wait_event_interruptible(threads_waitqueue, (turn == my_id));
    mutex_lock(&buff_m);
    sprintf(buff, "Thread %d is executing\n", my_id);
    mutex_unlock(&buff_m);
    complete(&available_data);
    msleep(500);
  }


  return 0;
}

static int scheduler_thread(void* arg)
{

  wait_event_interruptible(threads_waitqueue, (allready == thread_num));

  wake_up_interruptible_all(&sched_waitqueue);

  while(!kthread_should_stop())
  {
    turn = (turn + 1) % thread_num;
    wake_up_interruptible_all(&threads_waitqueue);
    msleep(2000);
  }

  return 0;

}

int scheduler_create(int thread_num, double period)
{
  //TODO:initialise all the mutexes here
  out_id = (struct task_struct**)kmalloc(thread_num * sizeof(struct task_struct*), GFP_USER);

  mutex_init(&buff_m);
  mutex_init(&ready_mutex);
  init_completion(&available_data);
  init_waitqueue_head(&sched_waitqueue);
  init_waitqueue_head(&threads_waitqueue);

  sched_id = kthread_run(scheduler_thread, NULL, "sched_thread");
  if (IS_ERR(out_id))
  {
    printk("Error creating scheduler thread!\n");
    return PTR_ERR(out_id);
  }

  return 0;
}

int thread_create(int id)
{ 
  int * tmp = (int*)kmalloc(sizeof(int),GFP_USER);
  *tmp = id;

  out_id[id] = kthread_run(output_thread, (void*)tmp,"out_thread");
  if (IS_ERR(out_id)) {
    printk("Error creating kernel thread!\n");

    return PTR_ERR(out_id);
  }

  return 0;
}

void thread_destroy(int id)
{
  kthread_stop(out_id[id]);
}

void scheduler_destroy()
{
  kthread_stop(sched_id);
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
