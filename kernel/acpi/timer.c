#include "acpi/acpi.h"
#include "acpi/lapic.h"
#include "acpi/timer.h"

#include <cpu/pit.h>
#include <stdio.h>

void apic_enable_timer()
{
  uint32_t initial_state = lapic_read(LAPIC_TIMER_REGISTER);
  lapic_write(LAPIC_TIMER_DIVIDE_CONFIG_REGISTER, 0x3); // set divider to 16
  lapic_write(LAPIC_TIMER_INITIAL_COUNT_REGISTER, 0xFFFFFFFF);
  lapic_write(LAPIC_TIMER_REGISTER, (initial_state & ~(LAPIC_INTERRUPT_MASK)) | LAPIC_TIMER_IRQ | LAPIC_TIMER_MODE_PERIODIC);
  pit_sleep(100);

  lapic_write(LAPIC_TIMER_REGISTER, initial_state | LAPIC_INTERRUPT_MASK);
  uint32_t ticks = 0xFFFFFFFF - lapic_read(LAPIC_TIMER_CURRENT_COUNT_REGISTER);
  printf("Timer ticked %d times", ticks);
  lapic_write(LAPIC_TIMER_REGISTER, (initial_state & ~(LAPIC_INTERRUPT_MASK)) | LAPIC_TIMER_IRQ | LAPIC_TIMER_MODE_PERIODIC);
  lapic_write(LAPIC_TIMER_DIVIDE_CONFIG_REGISTER, 0x3);
  lapic_write(LAPIC_TIMER_INITIAL_COUNT_REGISTER, ticks / 100000); // divide ticks by 100 to get milliseconds then divide by a thousand to get microseconds
}
