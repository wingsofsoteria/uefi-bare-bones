#include <acpi/pic.h>
#include <config.h>
#include <cpu/isr.h>
#include <cpu/tsc.h>
#include <cpu/pit.h>
#include <acpi/acpi.h>
#include <cpu/gdt.h>
#include <cpu/idt.h>
#include <cpu/task.h>
#include <cpu/sleep.h>
#include <graphics/pixel.h>
#include <graphics/tty.h>
#include <memory/alloc.h>
#include <keyboard.h>
#include <types.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

// BTW if anyone ever actually tries to read this code, it might be some of the
// worst code I have ever or will ever write
// my methodology for actually writing this damn thing was (random burst of
// inspiration at like 2 am) ' man that would be a great thing to add, let me
// just whip up something quick and I'll fix it later ' (queue 10 years of this
// thing in one form or another and this is what was born of it)
// I also only ever comment things after the fact so that doesn't help either

// TODO theres a bug *somewhere* that seems to be caused by printing too many
// lines to the screen

// TODO syscalls, porting a c library, better interrupt handling, actually
// support framebuffer formats instead of assuming 32bpp
// TODO use cpuid to calibrate TSC if available
// TODO have abort dump the task stack data structures
// TODO finish handling keyboard tasks + basic shell
// TODO finish TSC for APIC timer
// TODO finish AML interpreter OR switch to ACPICA
// TODO locking mechanism for tasks + better scheduling (right now I have little
// ability to quickly edit the tasks in queue since it requires looking through
// the ENTIRE list starting at the idle task)
void idle(void* _inner)
{
  printf("KERNEL IDLE");
  for (;;)
  {
  }
}
void task_1(void* _inner)
{
  printf("TASK 1");
  for (;;)
  {
  }
}

static void print_cpu_flags()
{
  uint64_t eax = 0;
  uint64_t ecx = 0;
  uint64_t edx = 0;
  uint64_t ebx = 0;
  asm volatile("mov $0x01, %%rax\n"
               "cpuid\n"
               "mov %%rax, %0\n"
               "mov %%rcx, %1\n"
               "mov %%rdx, %2\n"
               "mov %%rbx, %3\n"
    : "=m"(eax), "=m"(ecx), "=m"(edx), "=m"(ebx)
    :
    : "rax", "rcx", "rdx", "rbx", "memory");

  int tsc_available = ecx & (1 << 24);

  printf("CPU Family: %x\nTSC available: %s\nAPIC has tsc deadline mode: %s\n",
    ((eax & 0xF00) >> 8) + ((eax & 0x0FF00000) >> 20),
    ((edx & 0b10000) ? "True" : "False"), tsc_available ? "True" : "False");

  eax = 0;
  ebx = 0;
  ecx = 0;
  asm volatile("mov $0x16, %%rax\n"
               "cpuid\n"
               "mov %%rax, %0\n"
               "mov %%rbx, %1\n"
               "mov %%rcx, %2\n"
    : "=m"(eax), "=m"(ebx), "=m"(ecx)
    :
    : "rax", "rcx", "rbx", "memory");

  if (eax != 0)
  {
    printf("TSC Freq Ratio: %x/%x\nCore Crystal Clock: %x", ebx, eax, ecx);
  }
  else
  {
  }
}

void test_task(void* data)
{
  uint64_t old_cursor = get_cursor();
  set_cursor(79, 1);
  tty_putc(1);
  set_cursor(old_cursor >> 32, old_cursor & 0xFFFFFFFF);
}

int _start(kernel_bootinfo_t* bootinfo, void* ptr)
{
  asm volatile("cli");
  kernel_config = 0;
  load_gdt();
  load_idt();
  init_fb(bootinfo->base, bootinfo->pitch, bootinfo->horizontal_resolution,
    bootinfo->vertical_resolution);
  clear_screen();
  setup_allocator(bootinfo->mmap);
  acpi_init(bootinfo->xsdt_address);
  clear_screen();
  enable_irq(1, 33, keyboard_isr);
  enable_tasking();
  // init_tasks();
  // create_task(idle);
  // create_task(task_1);
  calibrate_tsc_slow();
  enable_pit();
  init_kb_status();
  lapic_enable();
  print_cpu_flags();
  create_task(test_task, NULL);
  task_loop();
  // TODO finish aml parser
  // dsdt_parse();
  halt_cpu
}
