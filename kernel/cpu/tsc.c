#include "cpu/pit.h"
#include "cpu/tsc.h"

#include <stdint.h>
#include <stdio.h>

uint64_t rdtsc()
{
  uint64_t low;
  uint64_t high;
  asm volatile("rdtsc"
    : "=a"(low), "=d"(high));
  return ((high << 32) | low);
}

static uint64_t calibrate_tsc_single_pass()
{
  int ms         = 10;
  uint16_t ticks = (ms * PIT_FREQ) / 1000;

  outb(PIT_MODE_COMMAND_REG, 0x30);
  outb(PIT_DATA0, ticks & 0xff);
  outb(PIT_DATA0, ticks >> 8);

  uint64_t t1;
  uint64_t t2;
  uint64_t delta = 0;
  t1 = t2 = rdtsc();
  while (pit_count() > 0)
  {
    t2 = rdtsc();
  }

  delta = t2 - t1;
  return delta / ms;
}

// TODO try to speed this up
uint64_t calibrate_tsc_slow()
{
  asm volatile("cli");
  int acceptable_error = 500;
  uint64_t d0          = 0;
  uint64_t d1          = 0;
  uint64_t d2          = 0;

  for (;;)
  {
    d2 = d1;
    d1 = d0;
    d0 = calibrate_tsc_single_pass();

    int d_error_1 = d0 - d1;
    int d_error_2 = d0 - d2;
    if (d_error_1 > acceptable_error || d_error_1 < -acceptable_error ||
      d_error_2 > acceptable_error || d_error_2 < -acceptable_error)
    {
      continue;
    }
    uint64_t avg = (d0 + d1 + d2) / 3;
    printf("found valid tsc frequency in kHz %d, %d MHz, %d GHz\n", avg,
      avg / 1000, avg / 1000000);
    return avg;
  }
}

void set_tsc_deadline(uint64_t deadline)
{
  write_msr(0x6e0, deadline & 0xFFFFFFFF, deadline >> 32);
}
