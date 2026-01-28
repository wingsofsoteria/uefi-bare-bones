#include "cpu/idt.h"
#include <stdint.h>

void ksleep(uint64_t milliseconds)
{
  ticks = milliseconds;
  while (ticks > 0)
  {
    asm volatile("hlt");
  }
}
