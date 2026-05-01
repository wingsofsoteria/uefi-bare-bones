#include "cpu/tsc.h"
#include "cpu/pit.h"
#include "utils.h"

#include <stdint.h>
#include <stdio.h>

static uint64_t calibrate_tsc_single_pass()
{
  int      ms    = 10;
  uint16_t ticks = (ms * PIT_FREQ) / 1000;
  outb(PIT_MODE_COMMAND_REG, 0x30);
  outb(PIT_DATA0, ticks & 0xff);
  outb(PIT_DATA0, ticks >> 8);
  uint64_t t1;
  uint64_t t2;
  uint64_t delta = 0;
  t1 = t2     = rdtsc();
  uint8_t lsb = inb(0x40);
  uint8_t msb = inb(0x40);
  while (msb > 0) {
    t2  = rdtsc();
    lsb = inb(0x40);
    msb = inb(0x40);
  }
  delta = t2 - t1;
  return delta / ms;
}

// TODO try to speed this up
uint64_t calibrate_tsc_slow()
{
  int acceptable_error = 500;

  uint64_t d0 = 0;
  uint64_t d1 = 0;
  uint64_t d2 = 0;
  for (int i = 0; i < 500; i++) {
    d2 = d1;
    d1 = d0;
    d0 = calibrate_tsc_single_pass();

    int d_error_1 = (d0 - d1) < acceptable_error;
    int d_error_2 = (d0 - d2) < acceptable_error;
    if (d_error_1 && d_error_2) {
      uint64_t avg = (d0 + d1 + d2) / 3;
      printf("found valid tsc frequency in kHz %lu, %lu MHz, %lu GHz\n", avg,
             avg / 1000, avg / 1000000);
      return avg;
    }
  }
  return calibrate_tsc_single_pass();
}

void set_tsc_deadline(uint64_t deadline)
{
  write_msr(0x6e0, deadline & 0xFFFFFFFF, deadline >> 32);
}
