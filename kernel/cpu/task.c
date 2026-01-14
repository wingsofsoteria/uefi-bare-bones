#include "cpu/task.h"
#include "cpu/idt.h"

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <keyboard.h>

static task_t* current  = NULL;
static task_t* kernel   = NULL;
static task_t* keyboard = NULL;

void init_tasks()
{
  kernel                = calloc(1, sizeof(task_t));
  kernel->ctx           = calloc(1, sizeof(isr_stack_t));
  kernel->ctx->rip      = (uint64_t)idle;
  kernel->ctx->rflags   = 0x202;
  kernel->ctx->isr      = 2;
  kernel->ctx->err      = 0;
  kernel->ctx->cs       = 0x08;
  kernel->next          = kernel;
  keyboard              = calloc(1, sizeof(task_t));
  keyboard->ctx         = calloc(1, sizeof(isr_stack_t));
  keyboard->ctx->rip    = (uint64_t)kb_handle_key;
  keyboard->ctx->rflags = 0x202;
  keyboard->ctx->isr    = 2;
  keyboard->ctx->err    = 0;
  keyboard->ctx->cs     = 0x08;
  keyboard->next        = kernel;

  current = kernel;
}

isr_stack_t* copy_ctx(isr_stack_t* dest, isr_stack_t* src)
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

void switch_task_to_kb(isr_stack_t* ctx)
{
  current->ctx = copy_ctx(current->ctx, ctx);
  current      = keyboard;
  ctx          = copy_ctx(ctx, current->ctx);
}

void switch_task(isr_stack_t* ctx)
{
  if (current->next == NULL) return;
  if (current == NULL) return;
  current->ctx = copy_ctx(current->ctx, ctx);
  current      = current->next;
  ctx          = copy_ctx(ctx, current->ctx);
}

void create_task(task_function rip)
{
  task_t* new_task     = calloc(1, sizeof(task_t));
  isr_stack_t* ctx     = calloc(1, sizeof(isr_stack_t));
  new_task->next       = calloc(1, sizeof(task_t));
  ctx->rip             = (uint64_t)rip;
  ctx->rflags          = 0x202;
  ctx->isr             = 2;
  ctx->err             = 0;
  ctx->cs              = 0x08;
  new_task->ctx        = ctx;
  new_task->ctx_uninit = true;
  new_task->next       = kernel->next;
  kernel->next         = new_task;
}
