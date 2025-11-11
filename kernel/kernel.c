#include "acpi/acpi.h"
#include "acpi/ioapic.h"
#include "acpi/lapic.h"
#include "cpu/gdt.h"
#include "cpu/idt.h"
#include "graphics/pixel.h"
#include "graphics/tty.h"
#include "memory/alloc.h"
#include "stdlib.h"
#include "types.h"
#include <stdint.h>
#include <stdio.h>

#include "cpu/task.h"
// TODO syscalls, porting a c library, better interrupt handling, actually support framebuffer formats instead of assuming 32bpp

// TODO have abort dump the task stack data structures
// TODO finish handling keyboard tasks + basic shell
int kernel_initialization;
uint64_t kernel_ptr;
void finish_init()
{
  kernel_initialization = 0;
}
void idle()
{
  printf("KERNEL IDLE");
  for (;;)
  {
  }
}
void task_1()
{
  printf("TASK 1");
  for (;;)
  {
  }
}

int _start(kernel_bootinfo_t* bootinfo, void* ptr)
{
  asm volatile("cli");
  load_gdt();
  load_idt();
  kernel_initialization = -255;
  kernel_ptr            = (uint64_t)ptr;
  init_fb(bootinfo->base, bootinfo->pitch, bootinfo->horizontal_resolution, bootinfo->vertical_resolution);
  clear_screen();
  tty_putc(1);
  printf("Kernel Start %d\n", -1);
  printf("Starting Kernel\n");
  printf("Got magic value: %s\n", bootinfo->magic);
  printf("Framebuffer Base: %x\nFramebuffer Size: %x\n", bootinfo->base, bootinfo->size);
  setup_allocator(bootinfo->mmap);
  printf("%x\n", ptr);
  printf("Setting up ACPI\n");
  setup_acpi(bootinfo->xsdt_address);
  clear_screen();
  enable_ps2_keyboard();
  init_tasks();
  create_task(idle);
  create_task(task_1);
  lapic_enable();
  halt_cpu
}
