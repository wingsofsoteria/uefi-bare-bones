#include "kernel.h"

#include "cpu/tss.h"
#include "pci/pci.h"
#include "shell.h"
#include "utils.h"

#include <acpi/acpi.h>
#include <acpi/pic.h>
#include <config.h>
#include <cpu/gdt.h>
#include <cpu/idt.h>
#include <cpu/isr.h>
#include <cpu/pit.h>
#include <cpu/sleep.h>
#include <cpu/task.h>
#include <cpu/tsc.h>
#include <keyboard.h>
#include <loaders/loader.h>
#include <log.h>
#include <memory/alloc.h>
#include <terminal/pixel.h>
#include <terminal/tty.h>
#include <types.h>

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
static void test_usermode() {
  for (;;);
  asm volatile("cli"); // expect a GPF exception
}

static void common_init_start()
{
  asm volatile("cli");
  load_gdt();
  load_idt();
  init_tss();
  init_config_cpuid();
}

extern void bootloader_specific_init();
// NOLINTNEXTLINE
int kmain()
{
  common_init_start();
  bootloader_specific_init();

  klog("TSC: %d\n", kernel_config.timers.tsc_freq_khz);
  klog(
    "Kernel offsets\nHHDM Start %llx\nKernel Mapping %p - %p\nKernel Size: "
    "%td\n",
    hhdm_mapping,
    &_kernel_start_addr,
    &_kernel_end_addr,
    &_kernel_end_addr - &_kernel_start_addr
  );
  klog("Kernel finished initialization\n");
  enable_irq(1, 33, keyboard_isr);
  enable_tasking();
  enable_apic();
  enable_interrupts();
  acpi_late_init();
  init_pci();
  init_shell();
  while (kernel_config.kexit == 0) { asm volatile("hlt"); }

  klog("Kernel was told to exit, Goodbye!\n");
  shutdown();
  klog("UH OH");
  // we should never get here
  halt();
}
