#include "config.h"
#include "cpu/idt.h"
#include "cpu/tsc.h"
#include <stdint.h>

void ksleep(uint64_t duration)
{
  uint64_t deadline = rdtsc();
  ticks             = duration;
  if (kernel_config.timer_source & 0b10)
  {
    set_tsc_deadline(deadline + (duration * (kernel_config.tsc_freq_khz / 1)));
  }

  while (ticks > 0)
  {
    asm volatile("hlt");
  }
}
