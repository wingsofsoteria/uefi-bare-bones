#ifndef __KERNEL_CONFIG_H__
#define __KERNEL_CONFIG_H__

#include <stdint.h>
extern uint8_t interrupt_config;

#define INTERRUPT_CONFIG_APIC    0b10
#define INTERRUPT_CONFIG_PIC     0b01
#define INTERRUPT_CONFIG_INVALID 0b11

#endif
