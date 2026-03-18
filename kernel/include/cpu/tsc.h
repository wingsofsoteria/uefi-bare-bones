#ifndef __KERNEL_CPU_TSC_H__
#define __KERNEL_CPU_TSC_H__

#include <stdint.h>
uint64_t rdtsc(void);

uint64_t calibrate_tsc_slow();
void set_tsc_deadline(uint64_t);
#endif
