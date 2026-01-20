#include <acpi/acpi.h>
#include <acpi/lapic.h>
#include <cpu/gdt.h>
#include <cpu/idt.h>
#include <cpu/task.h>
#include <graphics/pixel.h>
#include <graphics/tty.h>
#include <memory/alloc.h>
#include <keyboard.h>
#include <types.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
// TODO syscalls, porting a c library, better interrupt handling, actually support framebuffer formats instead of assuming 32bpp

// TODO have abort dump the task stack data structures
// TODO finish handling keyboard tasks + basic shell
// TODO move lapic code out of start function

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

int _start(kernel_bootinfo_t* bootinfo, void* ptr)
{
  asm volatile("cli");
  load_gdt();
  load_idt();
  init_fb(bootinfo->base, bootinfo->pitch, bootinfo->horizontal_resolution, bootinfo->vertical_resolution);
  clear_screen();
  setup_allocator(bootinfo->mmap);
  acpi_init(bootinfo->xsdt_address);
  clear_screen();
  enable_irq(1, 33);
  // init_tasks();
  // create_task(idle);
  // create_task(task_1);
  init_kb_status();
  outb(0x70, 0x8B);
  char value = inb(0x71);
  outb(0x70, 0x8B);
  outb(0x71, value | 0x40);
  lapic_enable();
  halt_cpu
}
