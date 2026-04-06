#include "shell.h"
#include "stdlib.h"
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
#include <loaders/loader.h>
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

extern void kernel_init_code();

void common_init_start()
{
  asm volatile("cli");
  load_gdt();
  load_idt();
}

#ifdef KERNEL_USE_LIMINE

__attribute__((used,
  section(
    ".limine_requests"))) static volatile uint64_t limine_base_revision[] =
  LIMINE_BASE_REVISION(6);

__attribute__((used,
  section(
    ".limine_requests"))) static volatile struct limine_framebuffer_request
  framebuffer_request = {
    .id       = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0,
};

__attribute__((used,
  section(".limine_requests"))) static volatile struct limine_memmap_request
  memmap_request = {.id = LIMINE_MEMMAP_REQUEST_ID, .revision = 0};

struct Frame next_usable(void* memory_map_ptr)
{
  struct limine_memmap_response* memory_map = memory_map_ptr;
  struct limine_memmap_entry** entries      = memory_map->entries;
  for (int i = 0; i < memory_map->entry_count; i++)
  {
    struct limine_memmap_entry* entry = entries[i];
    if (entry->type == LIMINE_MEMMAP_USABLE)
    {
      LOG_DEBUG(
        "Valid entry found: addr: %x size: %d\n", entry->base, entry->length);
      return (struct Frame){
        .start = false, .base = entry->base, .length = entry->length};
    }
  }
  return (struct Frame){.start = true, .base = 0, .length = 0};
}

// NOLINTNEXTLINE
int _start()
{
  if (LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision) == false)
  {
    halt_cpu;
  }
  if (framebuffer_request.response == NULL ||
    framebuffer_request.response->framebuffer_count < 1)
  {
    halt_cpu;
  }
  if (memmap_request.response == NULL)
  {
    halt_cpu;
  }
  struct limine_framebuffer* framebuffer =
    framebuffer_request.response->framebuffers[0];
  if (framebuffer->bpp != 32)
  {
    halt_cpu;
  }
  common_init_start();
  init_fb((uint64_t)framebuffer->address, framebuffer->pitch,
    framebuffer->width, framebuffer->height);
  LOG_DEBUG("Testing Limine Loader\n");
  setup_allocator(next_usable, memmap_request.response);
  halt_cpu;
}
#else
// NOLINTNEXTLINE
int _start(kernel_bootinfo_t* bootinfo, void* ptr)
{
  common_init_start();
  init_fb(bootinfo->base, bootinfo->pitch, bootinfo->horizontal_resolution,
    bootinfo->vertical_resolution);
  kernel_rsdp_from_bootinfo(bootinfo);
  init_config_cpuid();
  setup_allocator(bootinfo->mmap);

  uint64_t rsp = 0;
  asm volatile("mov %%rsp, %0"
    : "=a"(rsp));
  printf(
    "provided stack space:\n\tTOP: %x\n\tBOTTOM: %x\n\tSIZE: %l\nactive stack space:\n\tTOP: %x\n\tBOTTOM: %x\n\tSIZE: %l\n",
    bootinfo->stack_top, bootinfo->stack_bottom,
    bootinfo->stack_top - bootinfo->stack_bottom, rsp, bootinfo->stack_bottom,
    rsp - bootinfo->stack_bottom);

  // kernel_init();
  clear_screen();
  acpi_init(0);
  halt_cpu;
  enable_irq(1, 33, keyboard_isr);
  enable_tasking();
  // enable_pit();
  enable_apic();
  enable_interrupts();
  init_shell();
  while (kernel_config.kexit == 0)
  {
    asm volatile("hlt");
  }

  printf("Kernel was told to exit, Goodbye!\n");
  asm volatile("cli");
  return 1;
}
#endif
