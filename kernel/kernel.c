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
#include <stdio.h>
#include "cpu/tasking.h"
// TODO syscalls, porting a c library, better interrupt handling, actually support framebuffer formats instead of assuming 32bpp

// TODO multitasking setup currently causes Invalid Opcode Exception
// TODO have abort dump the task stack data structures
// TODO we should probably move ALL of the tasking functions to assembly to avoid the compiler changing register values (there is probably some attribute for this but its better to have full control)
int kernel_initialization;
void finish_init()
{
  kernel_initialization = 0;
}
int _start(kernel_bootinfo_t* bootinfo)
{
  asm volatile("cli");
  load_gdt();
  load_idt();
  kernel_initialization = -255;
  init_fb(bootinfo->base, bootinfo->pitch, bootinfo->horizontal_resolution, bootinfo->vertical_resolution);
  clear_screen();
  tty_putc(1);
  printf("Kernel Start %d\n", -1);
  printf("Starting Kernel\n");
  printf("Got magic value: %s\n", bootinfo->magic);
  printf("Framebuffer Base: %x\nFramebuffer Size: %x\n", bootinfo->base, bootinfo->size);
  setup_allocator(bootinfo->mmap);

  init_tasking();  
//  create_task(Task1, 0);
//  create_task(Task2, 1);
  printf("Setting up ACPI\n");
  setup_acpi(bootinfo->xsdt_address);
  enable_ps2_keyboard();
	lapic_enable();
  halt_cpu
}
