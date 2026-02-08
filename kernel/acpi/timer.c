#include "acpi.h"
#include "acpi/pic.h"

#include <cpu/pit.h>
#include <config.h>
#include <cpu/idt.h>

void apic_sleep(uint64_t duration)
{
  ticks = duration;

  while (ticks > 0)
  {
    asm volatile("hlt");
  }
}

void apic_enable_timer()
{
  uint32_t initial_state = lapic_read(LAPIC_TIMER_REGISTER);
  lapic_write(LAPIC_TIMER_DIVIDE_CONFIG_REGISTER, 0x3); // set divider to 16
  lapic_write(LAPIC_TIMER_INITIAL_COUNT_REGISTER, 0xFFFFFFFF);
  lapic_write(LAPIC_TIMER_REGISTER,
    (initial_state & ~(LAPIC_INTERRUPT_MASK)) | LAPIC_TIMER_IRQ |
      LAPIC_TIMER_MODE_PERIODIC);
  pit_sleep(10);

  lapic_write(LAPIC_TIMER_REGISTER, initial_state | LAPIC_INTERRUPT_MASK);
  uint32_t ticks_passed =
    0xFFFFFFFF - lapic_read(LAPIC_TIMER_CURRENT_COUNT_REGISTER);
  lapic_write(LAPIC_TIMER_REGISTER,
    (initial_state & ~(LAPIC_INTERRUPT_MASK)) | LAPIC_TIMER_IRQ |
      LAPIC_TIMER_MODE_PERIODIC);
  lapic_write(LAPIC_TIMER_DIVIDE_CONFIG_REGISTER, 0x3);
  lapic_write(LAPIC_TIMER_INITIAL_COUNT_REGISTER,
    ticks_passed / 10); // divide ticks by 10 to get milliseconds

  kernel_config |= TIMER_CONFIG_APIC_TIMER_ENABLED;
  kernel_config &= ~(TIMER_CONFIG_PIT_ENABLED);
}
