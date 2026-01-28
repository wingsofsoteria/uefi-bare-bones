#ifndef __KERNEL_CPU_PIT_H__
#define __KERNEL_CPU_PIT_H__

#include <stdint.h>
void pit_init();
void pit_sleep(uint64_t);

#define FREQ             1193182
#define HZ               1000
#define MODE_COMMAND_REG 0x43
#define DATA0            0x40

#endif
