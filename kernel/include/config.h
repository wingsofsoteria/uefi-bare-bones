#ifndef __KERNEL_CONFIG_H__
#define __KERNEL_CONFIG_H__

#include <stdint.h>
#define KERNEL_DEBUG             0b11110000
#define INTERRUPT_CONFIG_PIC     0b00000001
#define INTERRUPT_CONFIG_APIC    0b00000010
#define INTERRUPT_CONFIG_TASKING 0b00000100
#define TIMER_CONFIG_PIT         0b0000000100000000
#define TIMER_CONFIG_APIC_TIMER  0b0000001000000000
extern uint16_t TICK_RATE;

void enable_tasking();
void enable_pit();
void enable_apic();
uint16_t is_flag_enabled(uint16_t);
void enable_flag(uint16_t);
void init_kernel_config();
uint16_t is_debug();
void check_cpuid();
#endif
