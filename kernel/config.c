#include <stdint.h>
#include "config.h"
#include "acpi/pic.h"
#include "cpu/isr.h"
#include "cpu/pit.h"
#include "cpu/task.h"
static uint16_t kernel_config;
uint16_t TICK_RATE;
void enable_tasking()
{
  init_tasks();
  kernel_config |= INTERRUPT_CONFIG_TASKING;
}

void enable_pit()
{
  pit_init();
  enable_irq(0, 34, pic_timer_isr);
  kernel_config |= TIMER_CONFIG_PIT;
  TICK_RATE      = 1;
}

void enable_flag(uint16_t bits)
{
  kernel_config |= bits;
}

uint16_t is_flag_enabled(uint16_t bits)
{
  return kernel_config & bits;
}
void enable_apic()
{
  register_handler(32, apic_timer_isr);
  apic_enable_timer();
  disable_irq(0, 34);

  kernel_config &= ~(TIMER_CONFIG_PIT);
  kernel_config |= TIMER_CONFIG_APIC_TIMER;
  kernel_config |= INTERRUPT_CONFIG_APIC;
  TICK_RATE      = 1000;
}

uint16_t is_debug()
{
  return kernel_config & KERNEL_DEBUG;
}

void init_kernel_config()
{
  TICK_RATE = 0;
#ifdef QEMU_DEBUG
  kernel_config = KERNEL_DEBUG;
#endif
}
