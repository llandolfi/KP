#ifndef __EX_THREAD_H__
#define __EX_THREAD_H__

void thread_destroy(int id);
void scheduler_destroy(void);
int scheduler_create(int thread_num, double period);
int thread_create(int id);
int get_data(char *p, int size);
extern struct completion available_data;
extern int thread_num;
#endif	/* __EX_THREAD_H__ */