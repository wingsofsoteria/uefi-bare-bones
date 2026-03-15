#include "cpu/task.h"
#include "cpu/idt.h"
#include "cpu/isr.h"
#include "cpu/sleep.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <keyboard.h>

static task_t* current = NULL;
static task_t* kernel  = NULL;

static int TASK_COUNT;

static task_queue_t* queue = NULL;

static void push_queue(task_t* task)
{
  if (((queue->head + 1) % queue->capacity) == queue->tail)
  {
    return; // queue is full
  }
  queue->_inner[queue->tail] = task;
  queue->tail                = (queue->tail + 1) % queue->capacity;
}

static task_t* pop_queue()
{
  if (queue->head == queue->tail)
  {
    return kernel;
  }
  task_t* task = queue->_inner[queue->head];
  queue->head  = (queue->head + 1) % queue->capacity;
  return task;
}

// the kernel task is just meant to be a halt loop to prevent the task switching
// code from randomly dying
void init_tasks()
{
  register_handler(128, task_switch_isr);
  queue               = malloc(sizeof(task_queue_t));
  queue->head         = 0;
  queue->tail         = 0;
  queue->capacity     = TASK_QUEUE_SIZE;
  queue->_inner       = malloc(queue->capacity * sizeof(task_t*));
  TASK_COUNT          = 0;
  kernel              = calloc(1, sizeof(task_t));
  kernel->ctx         = calloc(1, sizeof(isr_stack_t));
  kernel->ctx->rip    = (uint64_t)idle;
  kernel->ctx->rflags = 0x202;
  kernel->ctx->isr    = 2;
  kernel->ctx->err    = 0;
  kernel->ctx->cs     = 0x08;
  kernel->task_id     = TASK_COUNT++;
  kernel->finished    = 0;
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
  current->ctx = copy_ctx(current->ctx, ctx);
  task_t* old  = current;
  current      = pop_queue();
  while (current->finished == 1)
  {
    free(current->ctx);
    free(current);
    current = pop_queue();
  }
  push_queue(old);
  ctx = copy_ctx(ctx, current->ctx);
}

static void remove_task(int id)
{
  if (id == 0) // id is 0 for the kernel / idle task so we never want to remove
               // that on accident
  {
    return;
  }
  for (int i = 0; i < queue->capacity; i++)
  {
    if (queue->_inner[i]->task_id == id)
    {
      queue->_inner[i]->finished = 1;
    }
  }
  // remove this task from the list and free all its structures
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
  push_queue(new_task);
}

void task_loop()
{
  while (1)
  {
    ksleep(10);
    asm volatile("int $128");
  }
}
