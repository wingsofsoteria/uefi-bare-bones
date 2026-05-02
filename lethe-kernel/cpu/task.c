#include "cpu/task.h"

#include "cpu/idt.h"
#include "cpu/isr.h"
#include "cpu/sleep.h"
#include "memory/alloc.h"
#include "utils.h"

#include <keyboard.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

static task_t* current = NULL;
static task_t* kernel  = NULL;

static int           TASK_COUNT;
static task_queue_t* LIVE_QUEUE = NULL;
static task_queue_t* IDLE_QUEUE = NULL;

static void idle()
{
  while (1) { asm volatile("hlt"); }
}

static void push_queue(task_queue_t* queue, task_t* task)
{
  if (((queue->head + 1) % queue->capacity) == queue->tail)
    {
      return; // queue is full
    }
  queue->_inner[queue->tail] = task;
  queue->tail                = (queue->tail + 1) % queue->capacity;
}

static task_t* pop_queue(task_queue_t* queue)
{
  if (queue->head == queue->tail) { return kernel; }
  task_t* task = queue->_inner[queue->head];
  queue->head  = (queue->head + 1) % queue->capacity;
  if (task == NULL) { return pop_queue(queue); }
  return task;
}

static task_queue_t* new_queue()
{
  task_queue_t* queue = kmalloc(sizeof(task_queue_t));
  queue->head         = 0;
  queue->tail         = 0;
  queue->capacity     = TASK_QUEUE_SIZE;
  queue->_inner       = kmalloc(queue->capacity * sizeof(task_t*));

  return queue;
}

int get_task_id() { return current->task_id; }

// the kernel task is just meant to be a halt loop to prevent the task switching
// code from randomly dying
void init_tasks()
{
  register_handler(128, task_switch_isr);
  LIVE_QUEUE          = new_queue();
  IDLE_QUEUE          = new_queue();
  TASK_COUNT          = 0;
  kernel              = kmalloc(sizeof(task_t));
  kernel->ctx         = kmalloc(sizeof(isr_stack_t));
  kernel->ctx->rip    = (uint64_t)idle;
  kernel->ctx->rflags = 0x202;
  kernel->ctx->isr    = 2;
  kernel->ctx->err    = 0;
  kernel->ctx->cs     = 0x08;
  kernel->task_id     = TASK_COUNT++;
  kernel->deadline    = 0;
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
  current      = pop_queue(LIVE_QUEUE);
  push_queue(LIVE_QUEUE, old);
  ctx = copy_ctx(ctx, current->ctx);
}

static task_t* remove_task(task_queue_t* queue, int id)
{
  if (id == 0) // id is 0 for the kernel / idle task so we never want to remove
               // that on accident
    {
      return NULL;
    }
  for (int i = 0; i < queue->capacity; i++)
    {
      if (queue->_inner[i]->task_id != id) { continue; }
      task_t* task     = queue->_inner[i];
      queue->_inner[i] = NULL;
      return task;
    }
  return NULL;
}

void signal_idle(uint64_t deadline)
{
  task_t* self = remove_task(LIVE_QUEUE, get_task_id());
  if (self == NULL) { return; }
  self->deadline = deadline;
  push_queue(IDLE_QUEUE, self);
}

// TODO implement this
// it'll require keeping track of what each idle task is waiting for
void signal_live() { abort(); }

// I have zero confidence in this btw
// the idea is that when the task function finishes it automatically removes
// itself from the task list, that way I can have a task run AND exit instead of
// just getting zombified
static void task_wrapper(int id, task_function fn_ptr, void* fn_args)
{
  fn_ptr(fn_args);
  task_t* self = remove_task(LIVE_QUEUE, id);
  if (self == NULL) { return; }
  kfree(self->ctx);
  kfree(self);
}

void create_task(task_function rip, void* data)
{
  task_t*      new_task = kmalloc(sizeof(task_t));
  isr_stack_t* ctx      = kmalloc(sizeof(isr_stack_t));
  new_task->task_id     = TASK_COUNT++;
  new_task->deadline    = 0;
  ctx->rdi              = new_task->task_id;
  ctx->rsi              = (uint64_t)rip;
  ctx->rdx              = (uint64_t)data;
  ctx->rip              = (uint64_t)task_wrapper;
  ctx->rflags           = 0x202;
  ctx->isr              = 2;
  ctx->err              = 0;
  ctx->cs               = 0x08;
  new_task->ctx         = ctx;
  push_queue(LIVE_QUEUE, new_task);
}

void task_loop()
{
  while (1)
    {
      ksleep((kernel_duration_t){ .milliseconds = 20 });
      asm volatile("int $128");
    }
}
