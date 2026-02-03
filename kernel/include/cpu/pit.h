#ifndef __KERNEL_CPU_PIT_H__
#define __KERNEL_CPU_PIT_H__

#include <stdint.h>
void pit_init();
void pit_sleep(uint64_t);
uint16_t pit_count();
#define PIT_FREQ             1193182
#define PIT_HZ               1000
#define PIT_MODE_COMMAND_REG 0x43
#define PIT_DATA0            0x40

#endif
