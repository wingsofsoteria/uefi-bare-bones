#ifndef __KERNEL_CPU_TSC_H__
#define __KERNEL_CPU_TSC_H__

#include <stdint.h>
#define NS_MULTIPLIER (TSC_FREQ_KHZ / 1000000)
uint64_t rdtsc(void);

void calibrate_tsc_slow();
void set_tsc_deadline(uint64_t);
extern uint64_t TSC_FREQ_KHZ;
#endif
