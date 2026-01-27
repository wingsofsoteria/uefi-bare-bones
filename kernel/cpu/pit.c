#include "cpu/idt.h"
#include <stdio.h>
#include <stdint.h>
#define FREQ             1193182
#define HZ               1000
#define MODE_COMMAND_REG 0x43
#define DATA0            0x40

// Init PIT with about 1000hz (~1ms per irq) freq
void pit_init()
{
  uint16_t reload_value = FREQ / HZ;
  outb(MODE_COMMAND_REG,
    0b00110100); // Select channel 0 with access mode lobyte/hibyte and
                 // operating mode rate generator and use 16-bit binary mode
  outb(DATA0, reload_value & 0xFF);
  outb(DATA0, reload_value >> 8);
}

void pit_sleep(uint16_t millis)
{
  ticks = millis;
  while (ticks > 0)
  {
    asm volatile("hlt");
  }
}
