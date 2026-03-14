#ifndef __KERNEL_CONFIG_H__
#define __KERNEL_CONFIG_H__

#include "acpi/pic.h"
#include "cpu/idt.h"
#include "cpu/isr.h"
#include "cpu/pit.h"
#include "cpu/task.h"
#include <stdint.h>
extern uint16_t kernel_config;

#define INTERRUPT_CONFIG_PIC            0b00000001
#define INTERRUPT_CONFIG_APIC           0b00000010
#define INTERRUPT_CONFIG_ENABLE_TASKING 0b00000100
#define TIMER_CONFIG_PIT_ENABLED        0b0000000100000000
#define TIMER_CONFIG_APIC_TIMER_ENABLED 0b0000001000000000

static inline void enable_tasking()
{
  init_tasks();
  kernel_config |= INTERRUPT_CONFIG_ENABLE_TASKING;
}

static inline void enable_pit()
{
  pit_init();
  enable_irq(0, 34, pic_timer_isr);
  kernel_config |= TIMER_CONFIG_PIT_ENABLED;
}

static inline void enable_apic()
{
  register_handler(32, apic_timer_isr);
  apic_enable_timer();
  disable_irq(0, 34);

  kernel_config &= ~(TIMER_CONFIG_PIT_ENABLED);
  kernel_config |= TIMER_CONFIG_APIC_TIMER_ENABLED;
  kernel_config |= INTERRUPT_CONFIG_APIC;
}

#endif
