#include "cpu/task.h"
#include "cpu/idt.h"
#include "cpu/isr.h"
#include "cpu/pit.h"
#include "cpu/sleep.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <keyboard.h>

static task_t* current = NULL;
static task_t* kernel  = NULL;

static int TASK_COUNT;

// the kernel task is just meant to be a halt loop to prevent the task switching
// code from randomly dying
void init_tasks()
{
  register_handler(128, task_switch_isr);
  TASK_COUNT          = 0;
  kernel              = calloc(1, sizeof(task_t));
  kernel->ctx         = calloc(1, sizeof(isr_stack_t));
  kernel->ctx->rip    = (uint64_t)idle;
  kernel->ctx->rflags = 0x202;
  kernel->ctx->isr    = 2;
  kernel->ctx->err    = 0;
  kernel->ctx->cs     = 0x08;
  kernel->task_id     = TASK_COUNT++;
  kernel->next        = kernel;
  current             = kernel;
}

static isr_stack_t* copy_ctx(isr_stack_t* dest, isr_stack_t* src)
{
  dest->r15    = src->r15;
  dest->r14    = src->r14;
  dest->r13    = src->r13;
  dest->r12    = src->r12;
  dest->r11    = src->r11;
  dest->r10    = src->r10;
  dest->r9     = src->r9;
  dest->r8     = src->r8;
  dest->rbp    = src->rbp;
  dest->rdi    = src->rdi;
  dest->rsi    = src->rsi;
  dest->rdx    = src->rdx;
  dest->rcx    = src->rcx;
  dest->rbx    = src->rbx;
  dest->rax    = src->rax;
  dest->isr    = src->isr;
  dest->err    = src->err;
  dest->rip    = src->rip;
  dest->cs     = src->cs;
  dest->rflags = src->rflags;
  return dest;
}

void switch_task(isr_stack_t* ctx)
{
  if (current->next == NULL)
  {
    return;
  }
  if (current == NULL)
  {
    return;
  }
  current->ctx = copy_ctx(current->ctx, ctx);
  current      = current->next;
  ctx          = copy_ctx(ctx, current->ctx);
}

static void remove_task(int id)
{
  if (id == 0) // id is 0 for the kernel / idle task so we never want to remove
               // that on accident
  {
    return;
  }
  task_t* task   = kernel;
  task_t* parent = kernel;
  while (task->task_id != id)
  {
    parent = task;
    task   = task->next;
    if (task == NULL)
    {
      return;
    }
  }
  // remove this task from the list and free all its structures
  parent->next = task->next;
  free(task->ctx);
  free(task);
}
// I have zero confidence in this btw
// the idea is that when the task function finishes it automatically removes
// itself from the task list, that way I can have a task run AND exit instead of
// just getting zombified
static void task_wrapper(int id, task_function fn_ptr, void* fn_args)
{
  fn_ptr(fn_args);
  remove_task(id);
}

void create_task(task_function rip, void* data)
{
  task_t* new_task  = calloc(1, sizeof(task_t));
  isr_stack_t* ctx  = calloc(1, sizeof(isr_stack_t));
  new_task->task_id = TASK_COUNT++;
  ctx->rdi          = new_task->task_id;
  ctx->rsi          = (uint64_t)rip;
  ctx->rdx          = (uint64_t)data;
  ctx->rip          = (uint64_t)task_wrapper;
  ctx->rflags       = 0x202;
  ctx->isr          = 2;
  ctx->err          = 0;
  ctx->cs           = 0x08;
  new_task->ctx     = ctx;
  new_task->next    = kernel->next;
  kernel->next      = new_task;
}

void task_loop()
{
  while (true)
  {
    ksleep(10);
    asm volatile("int $128");
  }
}
