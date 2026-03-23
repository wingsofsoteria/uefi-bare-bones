#ifndef __KERNEL_CPU_SLEEP_H__
#define __KERNEL_CPU_SLEEP_H__

#include <stdint.h>

#define MS_PER_SECOND 1000
#define NS_PER_SECOND 1000000000
#define MS_PER_NS     1000000

// My timer resolutions are EITHER 1 millisecond or 1 nanosecond
// also store seconds for convenience
typedef struct
{
  uint64_t seconds;
  uint64_t milliseconds;
  uint64_t nanoseconds;

} kernel_duration_t;
void ksleep(kernel_duration_t);

#endif
