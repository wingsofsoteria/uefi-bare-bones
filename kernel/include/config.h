#ifndef __KERNEL_CONFIG_H__
#define __KERNEL_CONFIG_H__

#include <stdint.h>

/*
 * interrupt_source
 * 	0 disabled
 * 	1 pic
 * 	2 apic
 * 	3 reserved
 * timer_source
 * 	0 none
 * 	1 pic
 * 	2 apic + tsc
 * 	3 reserved
 * 	4 hpet (unimplemented)
 * */

struct kernel_config
{
  uint8_t kexit : 1;
  uint8_t interrupts_enabled : 1;
  uint8_t interrupt_source : 2;
  uint8_t apic_tsc_deadline : 1;
  uint8_t tsc_invariant : 1;
  uint32_t tsc_freq_khz;
  uint8_t multitasking_enabled : 1;
  uint8_t timer_source : 3;
} __attribute__((packed));

#define MAYBE_STI                              \
  if (kernel_config.interrupts_enabled == 0b1) \
  {                                            \
    sti();                                     \
  }

#define MAYBE_CLI                              \
  if (kernel_config.interrupts_enabled == 0b1) \
  {                                            \
    cli();                                     \
  }
void sti();
void cli();
void enable_interrupts();
void enable_tasking();
void enable_pit();
void enable_apic();
void init_config_cpuid();
extern struct kernel_config kernel_config;
#endif
