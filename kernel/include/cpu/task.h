#ifndef __KERNEL_CPU_TASK_H__
#define __KERNEL_CPU_TASK_H__

#include <cpu/idt.h>
#define TASK_QUEUE_SIZE 10

typedef void (*task_function)(void*);

void install_task(task_function fn_ptr, void* data);

typedef struct
{
  isr_stack_t* ctx;
  int task_id;
  int finished;
} task_t;

typedef struct
{
  task_t** _inner;
  int head;
  int tail;
  int capacity;

} task_queue_t;

void create_task(task_function, void*);

void switch_task(isr_stack_t*);
void init_tasks();
void task_loop();
extern void idle(void*);

#endif
