#include "shell.h"
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
// lines to the screen (maybe fixed by removing the cli / sti instructions?)

// TODO syscalls, porting a c library, better interrupt handling, actually
// support framebuffer formats instead of assuming 32bpp
// TODO have abort dump the task stack data structures
// TODO finish AML interpreter OR switch to ACPICA
// TODO locking mechanism for tasks + better scheduling (right now I have little
// ability to quickly edit the tasks in queue since it requires looking through
// the ENTIRE list starting at the idle task)
// NOLINTNEXTLINE
void idle(void* _inner)
{
  printf("KERNEL IDLE");
  for (;;)
  {
  }
}

// NOLINTNEXTLINE
void task_1(void* _inner)
{
  printf("TASK 1");
  for (;;)
  {
  }
}

// NOLINTNEXTLINE
static void test_task(void* data)
{
  uint64_t old_cursor = get_cursor();
  set_cursor(79, 1);
  tty_putc(1);
  set_cursor(old_cursor >> 32, old_cursor & 0xFFFFFFFF);
}

// NOLINTNEXTLINE
int _start(kernel_bootinfo_t* bootinfo, void* ptr)
{
  asm volatile("cli");
  load_gdt();
  load_idt();
  init_fb(bootinfo->base, bootinfo->pitch, bootinfo->horizontal_resolution,
    bootinfo->vertical_resolution);
  clear_screen();
  init_config_cpuid();
  setup_allocator(bootinfo->mmap);
  init_kb_status();
  acpi_init(bootinfo->xsdt_address);
  enable_irq(1, 33, keyboard_isr);
  enable_tasking();
  // enable_pit();
  enable_apic();
  enable_interrupts();
  printf("TSC TEST\n");
  ksleep((kernel_duration_t){.seconds = 10});
  printf("TEST DONE\n");
  init_shell();

  while (kernel_config.kexit == 0)
  {
    asm volatile("hlt");
  }

  printf("Kernel was told to exit, Goodbye!\n");
  asm volatile("cli");
  return 1;
}
