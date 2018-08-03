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
static struct mutex list_mutex;

static wait_queue_head_t sched_waitqueue;
static wait_queue_head_t threads_waitqueue;

static int allready = 0;
static bool sched_ready = false;
static int turn = -1;

static char buff[1024];

struct node {
    int id;
    struct task_struct* value;
    struct list_head kl;
};

static struct list_head head;


static int output_thread(void *arg)
{

  int* my_id_p = (int*)arg;
  int my_id = *my_id_p;

  mutex_lock(&ready_mutex);
  allready = allready + 1;
  mutex_unlock(&ready_mutex);

  if (allready == thread_num)
  {
    printk("I am thread %d and I am the last thread, I am waking up all!\n", my_id);
    wake_up_interruptible_all(&threads_waitqueue);
  }
  else
  {
    printk("I am thread %d... I wait for all to be ready...\n", my_id);
    wait_event_interruptible(threads_waitqueue, (allready == thread_num));
  }

  printk("Thread %d waiting for the scheduler to be ready...\n", my_id);
  wait_event_interruptible(sched_waitqueue, (sched_ready == true));
  printk("Thread %d starting main body\n", my_id);

  while (!kthread_should_stop()) {
    wait_event_interruptible(threads_waitqueue, (turn == my_id));

    #ifdef MYDEBUG
      printk("Thread %d is executing\n", my_id);
    #endif


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
  printk("Scheduler started! Waiting for the other threads\n");
  wait_event_interruptible(threads_waitqueue, (allready == thread_num));

  printk("waking up all the threads?\n");

  mutex_lock(&ready_mutex);
  sched_ready = true;
  mutex_unlock(&ready_mutex);

  wake_up_interruptible_all(&sched_waitqueue);

  printk("Scheduler starting main body\n");
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
  mutex_init(&list_mutex);
  init_completion(&available_data);
  INIT_LIST_HEAD(&head);

  init_waitqueue_head(&sched_waitqueue);
  init_waitqueue_head(&threads_waitqueue);

  sched_id = kthread_run(scheduler_thread, NULL, "sched_thread");
  if (IS_ERR(sched_id))
  {
    printk("Error creating scheduler thread!\n");
    return PTR_ERR(sched_id);
  }

  return 0;
}

int thread_create(int id)
{ 
  int * tmp = (int*)kmalloc(sizeof(int),GFP_USER);
  *tmp = id;

  printk("Creating thread %d\n", *tmp);

  out_id[id] = kthread_run(output_thread, (void*)tmp,"out_thread");
  if (IS_ERR(out_id[id])) {
    printk("Error creating kernel thread!\n");

    return PTR_ERR(out_id[id]);
  }

  return 0;
}

int thread_create_list(int id)
{
  struct node *n;
  int * tmp = (int*)kmalloc(sizeof(int),GFP_USER);
  *tmp = id;

  printk("Creating thread %d\n", *tmp);

  n = kmalloc(sizeof(struct node), GFP_KERNEL);
  n->value = kthread_run(output_thread, (void*)tmp,"out_thread");
  n->id = id;
  list_add(&(n->kl), &head);

  if (IS_ERR(n->value)) {
    printk("Error creating kernel thread!\n");

    return PTR_ERR(n->value);
  }

  return 0;
}


void thread_destroy(int id)
{
  kthread_stop(out_id[id]);
}


void scheduler_destroy_list()
{
  struct list_head *l, *tmp;
  struct node *n;
  int i = 0;
  int j = 0;

  list_for_each_safe(l, tmp, &head) {
      n = list_entry(l, struct node, kl);

      for (j = 0; j < 3; j++)
      {
        turn = j;
        wake_up_interruptible_all(&threads_waitqueue);
      }

      kthread_stop(n->value);
      printk("Thread %d destroyed\n", n->id);
      
      list_del(l);
      kfree(n);

      i = i + 1;
  }

   kthread_stop(sched_id);
}

int sched_append_thread()
{

  mutex_lock(&list_mutex);

  thread_num = thread_num + 1;
  int res = thread_create_list(thread_num-1);

  mutex_unlock(&list_mutex);
  
  return res;
}

int sched_rm_thread()
{
  struct list_head *l, *tmp;
  struct node *n;

  mutex_lock(&list_mutex);

  printk("Removing thread %d\n", thread_num-1);

  list_for_each_safe(l, tmp, &head) {
      n = list_entry(l, struct node, kl);

      //Should enter the crytical section
      if (n->id == thread_num-1)
      {
        list_del(l);
        kfree(n);
        thread_num = thread_num - 1;
        break;
      }
  }

  mutex_unlock(&list_mutex);

  return 0;
}

void scheduler_destroy()
{
  int i = 0;
  for (i=0; i < thread_num; i++)
  {
    turn = i;
    wake_up_interruptible_all(&threads_waitqueue);
    kthread_stop(out_id[i]);
    printk("Thread %d destroyed\n", i);
  }

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
