#include "config.h"

#include "acpi/acpi.h"
#include "acpi/pic.h"
#include "cpu/idt.h"
#include "cpu/isr.h"
#include "cpu/pit.h"
#include "cpu/task.h"
#include "cpu/tsc.h"
#include "cpuid.h"
#include "log.h"

#include <stdint.h>
struct kernel_config kernel_config;

void init_config_cpuid()
{
  kernel_config      = (struct kernel_config){ 0 };
  uint32_t eax       = 0;
  uint32_t ebx       = 0;
  uint32_t ecx       = 0;
  uint32_t edx       = 0;
  uint32_t unused    = 0;
  int      supported = __get_cpuid(1, &eax, &ebx, &ecx, &edx);
  if (!supported)
    {
      kernel_log_error("Could not get CPU information\n");
      return;
    }
  if (!(edx & bit_MSR)) // I'm not sure this is even needed
    {
      kernel_log_error("MSRs unsupported\n");
    }

  if (ecx & bit_TSCDeadline) { kernel_config.apic_tsc_deadline = 0b1; }
  supported = __get_cpuid(0x80000007, &unused, &unused, &unused, &edx);
  if (supported && (edx & (1 << 8))) { kernel_config.tsc_invariant = 0b1; }

  supported = __get_cpuid(0x15, &eax, &ebx, &ecx, &edx);
  if (supported)
    {
      kernel_log_debug(
        "CPUID 0x15: eax %d, ebx %d, ecx %d, edx %d\n",
        eax,
        ebx,
        ecx,
        edx
      );
      // leaf 15 gives the tsc freq in hz so divide by 1000
      kernel_config.tsc_freq_khz = (ecx * (ebx / eax)) / 1000;
    }
  supported = __get_cpuid(0x16, &eax, &ebx, &ecx, &edx);
  if (supported)
    {
      kernel_log_debug(
        "CPUID 0x16: eax %d, ebx %d, ecx %d, edx %d\n",
        eax,
        ebx,
        ecx,
        edx
      );
      // leaf 16 gives the processor base frequency in mhz so multiply by 1000
      // for tsc frequency in khz
      kernel_config.tsc_freq_khz = eax * 1000;
    }
}

void cli()
{
  asm volatile("cli");
  if (kernel_config.interrupt_source & 0b10) { lapic_disable(); }
}

void sti()
{
  if (kernel_config.interrupt_source & 0b10) { lapic_enable(); }
  // TODO Legacy PIC support

  asm volatile("sti");
}

void enable_interrupts()
{
  sti();
  kernel_config.interrupts_enabled = 0b1;
}

void enable_tasking()
{
  MAYBE_CLI;
  init_tasks();
  kernel_config.multitasking_enabled = 0b1;
  MAYBE_STI;
}

void enable_pit()
{
  MAYBE_CLI;
  pit_init();
  enable_irq(0, 34, pic_timer_isr);
  kernel_config.timer_source |= 0b001;
  MAYBE_STI;
}

void enable_apic()
{
  MAYBE_CLI;

  // TODO faster tsc calibration
  if (kernel_config.tsc_freq_khz == 0)
    {
      if (
        kernel_config.interrupt_source != 0b10 ||
        !kernel_config.apic_tsc_deadline || !kernel_config.tsc_invariant
      )
        {
          kernel_log_error("APIC timer unsupported, switching to pit");
          enable_pit();
          return;
        }
      uint64_t frequency;
      int      error;
      frequency = calibrate_tsc_slow();
      error     = frequency - calibrate_tsc_slow();
      kernel_log_debug("CALIBRATION RESULT %lu %d\n", frequency, error);
      if (error > 1500 || error < -1500)
        {
          kernel_log_error("TSC is unreliable\n");
          return;
        }
      kernel_config.tsc_freq_khz = frequency;
    }
  register_handler(32, apic_timer_isr);
  apic_enable_timer();
  disable_irq(0, 34);
  // tell the kernel that the PIT is no longer usable for interrupts
  kernel_config.timer_source &= ~(0b001);
  kernel_config.timer_source |= 0b010;
  MAYBE_STI;
}
