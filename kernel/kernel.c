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

// TODO (priority) real time clock
// TODO syscalls, porting a c library, better interrupt handling, actually
// support framebuffer formats instead of assuming 32bpp
// TODO use cpuid to calibrate TSC if available
// TODO have abort dump the task stack data structures
// TODO finish handling keyboard tasks + basic shell
// TODO generic sleep function

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

void print_cpu_flags()
{
  uint64_t eax = 0, ecx = 0, edx = 0, ebx = 0;
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
  // init_tasks();
  // create_task(idle);
  // create_task(task_1);
  calibrate_tsc_slow();
  pit_init();
  enable_irq(0, 34, pic_timer_isr);
  init_kb_status();
  lapic_enable();
  register_handler(32, apic_timer_isr);
  apic_enable_timer();
  disable_irq(0, 34);

  print_cpu_flags();
  loop_print_definition_blocks();
  halt_cpu
}
