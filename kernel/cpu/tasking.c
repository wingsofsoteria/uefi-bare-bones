#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

//BROKEN
typedef struct
{
  uint64_t r15;
  uint64_t r14;
  uint64_t r13;
  uint64_t r12;
  uint64_t r11;
  uint64_t r10;
  uint64_t r9;
  uint64_t r8;
  uint64_t rbp;
  uint64_t rdi;
  uint64_t rsi;
  uint64_t rdx;
  uint64_t rcx;
  uint64_t rbx;
  uint64_t rax;
} __attribute__((packed)) x64_registers;

typedef struct
{
  uint16_t status;
  uint64_t rip;
  uint64_t rsp;
  x64_registers registers;
  uint16_t cs;
  uint16_t ds;
  uint16_t es;
  uint16_t fs;
  uint16_t gs;
  uint16_t ss;
  uint64_t cr3;
  uint64_t rflags;

} __attribute__((packed)) task_stack_t;

uint64_t num_tasks   = 0;
uint64_t active_task = 0;
uint64_t stack_allocation;
uint64_t cs;
uint64_t ret_addr;
uint64_t ss;
uint64_t stack_pointer;
uint64_t rflags;

task_stack_t task_array[3] = {0, 0, 0};
extern void* save_task_asm(void);
extern void load_task_asm(void*);
void save_task(int slot)
{
  uint64_t* data        = save_task_asm();
  task_stack_t* active  = &task_array[slot];
  active->registers.r15 = data[0];
  active->registers.r14 = data[1];
  active->registers.r13 = data[2];
  active->registers.r12 = data[3];
  active->registers.r11 = data[4];
  active->registers.r10 = data[5];
  active->registers.r9  = data[6];
  active->registers.r8  = data[7];
  active->registers.rbp = data[8];
  active->registers.rdi = data[9];
  active->registers.rsi = data[10];
  active->registers.rdx = data[11];
  active->registers.rcx = data[12];
  active->registers.rbx = data[13];
  active->registers.rax = data[14];
  active->cs            = cs;
  active->rip           = ret_addr;
  active->ss            = ss;
  active->rsp           = stack_pointer;
  active->rflags        = rflags;
}

void load_task()
{
  task_stack_t* active = &task_array[active_task];
    ret_addr            = active->rip;
   cs                  = active->cs;
   ss                  = active->ss;
   stack_pointer       = active->rsp;
   rflags              = active->rflags;
   load_task_asm(&(active->registers));
}

void create_task(void (*task_func)(void), int task_slot)
{
  asm("mov %%cs, %0\nmov %%ss, %1\npushfq\npop %2"
    : "=r"(cs), "=r"(ss), "=r"(rflags));
  num_tasks++;
  ret_addr          = (uint64_t)task_func;
  stack_allocation -= 1000;
  stack_pointer     = stack_allocation;
  save_task(task_slot);
}

void switch_task(int new_task)
{
  asm("cli");
  save_task(active_task);
  active_task = new_task;
  load_task();
  asm("sti");
}

void Task1()
{
  int i = 0;
  for (;; i++)
  {
    printf("Task 1 Test: %d\n", i);
  }
}

void Task2()
{
  int i = 0;
  for (;; i += 2)
  {
    printf("Task 2 Test: %d\n", i);
  }
}

void init_tasking()
{
  void* stack = malloc(0x4000);
  stack_allocation = (uint64_t)stack;
}
