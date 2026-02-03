#include "cpu/pit.h"
#include "cpu/tsc.h"

#include <stdint.h>
#include <stdio.h>

uint64_t TSC_FREQ_KHZ;

uint64_t rdtsc()
{
  uint64_t low, high;
  asm volatile("rdtsc"
    : "=a"(low), "=d"(high));
  return ((high << 32) | low);
}

uint64_t calibrate_tsc_single_pass()
{
  int ms         = 10;
  uint16_t ticks = (ms * PIT_FREQ) / 1000;

  outb(PIT_MODE_COMMAND_REG, 0x30);
  outb(PIT_DATA0, ticks & 0xff);
  outb(PIT_DATA0, ticks >> 8);

  uint64_t tsc, t1, t2;
  uint64_t delta = 0;
  t1 = t2 = rdtsc();
  while (pit_count() > 0)
  {
    t2 = rdtsc();
  }

  delta = t2 - t1;
  return delta / ms;
}

void calibrate_tsc_slow()
{
  int acceptable_error = 500;
  uint64_t d0, d1, d2;
  for (;;)
  {
    d2 = d1;
    d1 = d0;
    d0 = calibrate_tsc_single_pass();

    int d_error_1 = (d0 - d1) < acceptable_error;
    int d_error_2 = (d0 - d2) < acceptable_error;
    if (d_error_1 && d_error_2)
    {
      uint64_t avg = (d0 + d1 + d2) / 3;
      printf("found valid tsc frequency in kHz %d, %d MHz, %d GHz\n", avg,
        avg / 1000, avg / 1000000);
      TSC_FREQ_KHZ = avg;
      break;
    }
  }
}
