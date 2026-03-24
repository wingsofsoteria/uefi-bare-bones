// NOLINTBEGIN(misc-use-internal-linkage)
#include "cpu/sleep.h"
#include "config.h"
#include "cpu/idt.h"
#include "cpu/task.h"
#include "cpu/tsc.h"
#include <stdint.h>

static uint64_t duration_to_ns(kernel_duration_t duration)
{
  return duration.nanoseconds + (duration.milliseconds * MS_PER_NS) +
    (duration.seconds * NS_PER_SECOND);
}

static uint64_t duration_to_ms(kernel_duration_t duration)
{
  return duration.milliseconds + (duration.nanoseconds / MS_PER_NS) +
    (duration.seconds * MS_PER_SECOND);
}

void ksleep(kernel_duration_t duration)
{
  uint64_t deadline = rdtsc();
  if (kernel_config.timer_source & 0b10)
  {
    ticks          = 1;
    uint64_t nanos = duration_to_ns(duration);
    deadline = deadline + (nanos * kernel_config.tsc_freq_khz / MS_PER_NS);
    signal_idle(deadline);
    set_tsc_deadline(deadline);
  }
  else
  {
    ticks = duration_to_ms(duration);
  }
  while (ticks > 0)
  {
    asm volatile("hlt");
  }
}
// NOLINTEND(misc-use-internal-linkage)
