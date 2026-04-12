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
#include <terminal/pixel.h>
#include <terminal/tty.h>
#include <memory/alloc.h>
#include <keyboard.h>
#include <types.h>
#include <stdint.h>
#include <loaders/loader.h>
#include <log.h>
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
uint64_t hhdm_mapping = 0;

static void common_init_start()
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

__attribute__((
  used, section(".limine_requests"))) static volatile struct limine_hhdm_request
  hhdm_request = {.id = LIMINE_HHDM_REQUEST_ID, .revision = 0};

__attribute__((used,
  section(
    ".limine_requests"))) static volatile struct limine_paging_mode_request
  paging_mode_request = {.id = LIMINE_PAGING_MODE_REQUEST_ID,
    .revision                = 1,
    .mode                    = LIMINE_PAGING_MODE_X86_64_4LVL,
    .max_mode                = LIMINE_PAGING_MODE_X86_64_4LVL,
    .min_mode                = LIMINE_PAGING_MODE_X86_64_4LVL};

__attribute__((
  used, section(".limine_requests"))) static volatile struct limine_rsdp_request
  rsdp_request = {.id = LIMINE_RSDP_REQUEST_ID, .revision = 0};

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
  hhdm_mapping = 0;
  if (hhdm_request.response != NULL)
  {
    struct limine_hhdm_response* hhdm_response = hhdm_request.response;
    hhdm_mapping                               = hhdm_response->offset;
  }
  struct limine_framebuffer* framebuffer =
    framebuffer_request.response->framebuffers[0];
  if (framebuffer->bpp != 32)
  {
    halt_cpu;
  }
  if (rsdp_request.response == NULL)
  {
    halt_cpu;
  }

  common_init_start();
  init_fb((uint64_t)framebuffer->address, framebuffer->pitch,
    framebuffer->width, framebuffer->height);
  kernel_init_logging(KERNEL_LOG_DEBUG);
  kernel_log_debug("Kernel offset set to %x", hhdm_mapping);
  setup_allocator(memmap_request.response);
  kernel_init_code();
  init_config_cpuid();
  acpi_init(rsdp_request.response->address);
  kernel_log_debug("Kernel finished initialization");
  halt_cpu;
}
#else
// NOLINTNEXTLINE
int _start(kernel_bootinfo_t* bootinfo, void* ptr)
{
  common_init_start();
  init_fb(bootinfo->base, bootinfo->pitch, bootinfo->horizontal_resolution,
    bootinfo->vertical_resolution);
  init_config_cpuid();
  // setup_allocator(bootinfo->mmap);

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
