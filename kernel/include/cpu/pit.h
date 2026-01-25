#ifndef __KERNEL_CPU_PIT_H__
#define __KERNEL_CPU_PIT_H__

#include <stdint.h>
void pit_init();
void pit_sleep(uint64_t);
extern volatile uint16_t ticks;

#endif
