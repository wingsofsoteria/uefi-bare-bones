#ifndef __KERNEL_CPU_TASK_H__
#define __KERNEL_CPU_TASK_H__

#include "cpu/idt.h"
#include <stdbool.h>
typedef void (*task_function)(void*);

typedef struct task
{
  isr_stack_t* ctx;
  struct task* next;
  bool ctx_uninit;
} task_t;

void create_task(task_function);

void switch_task(isr_stack_t*);
void init_tasks();
#endif