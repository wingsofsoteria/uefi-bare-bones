#ifndef __KERNEL_CPU_TASK_H__
#define __KERNEL_CPU_TASK_H__

#include <cpu/idt.h>
#include <stdbool.h>

typedef void (*task_function)(void*);

void install_task(task_function fn_ptr, void* data);

typedef struct task
{
  isr_stack_t* ctx;
  struct task* next;
  int task_id;
} task_t;

void create_task(task_function, void*);

void switch_task(isr_stack_t*);
void init_tasks();
void task_loop();
extern void idle(void*);

#endif
