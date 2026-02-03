#ifndef __KERNEL_CPU_TSC_H__
#define __KERNEL_CPU_TSC_H__

#include <stdint.h>
uint64_t rdtsc(void);

void calibrate_tsc_slow();

extern uint64_t TSC_FREQ_KHZ;
#endif
