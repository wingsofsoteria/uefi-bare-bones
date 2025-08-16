#include "acpi/acpi.h"
#include "cpu/gdt.h"
#include "cpu/idt.h"
#include "graphics/pixel.h"
#include "graphics/tty.h"
#include "memory/alloc.h"
#include "memory/cpio.h"
#include "stdlib.h"
#include <stdio.h>

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
  init_cpio(bootinfo->initfs);
  uint8_t* font = get_file();
  init_text(font);
  test_pixels();
  // clear_screen();
  printf("Kernel Start %d\n", -1);
  printf("Starting Kernel\n");
  printf("Got magic value: %s\n", bootinfo->magic);
  printf("Framebuffer Base: %x\nFramebuffer Size: %x\n", bootinfo->base, bootinfo->size);
  setup_allocator(bootinfo->mmap);

  printf("Setting up ACPI\n");
  setup_acpi(bootinfo->xsdt_address);

  halt_cpu
}
