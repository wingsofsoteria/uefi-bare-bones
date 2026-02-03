#include "cpu/idt.h"
#include "cpu/pit.h"

#include <stdio.h>
#include <config.h>
#include <stdint.h>

uint16_t pit_count()
{
  uint16_t low, high;

  low  = inb(PIT_DATA0);
  high = inb(PIT_DATA0);

  return ((high << 8) | low);
}

// Init PIT with about 1000hz (~1ms per irq) freq
void pit_init()
{
  kernel_config         |= TIMER_CONFIG_PIT_ENABLED;
  uint16_t reload_value  = PIT_FREQ / PIT_HZ;
  outb(PIT_MODE_COMMAND_REG,
    0b00110100); // Select channel 0 with access mode lobyte/hibyte and
                 // operating mode rate generator and use 16-bit binary mode
  outb(PIT_DATA0, reload_value & 0xFF);
  outb(PIT_DATA0, reload_value >> 8);
}

void pit_sleep(uint64_t duration)
{
  ticks = duration;
  while (ticks > 0)
  {
    asm volatile("hlt");
  }
}
