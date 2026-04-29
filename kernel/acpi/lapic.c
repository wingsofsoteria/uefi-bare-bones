// TODO properly configure LAPIC

#include "acpi.h"
#include "stdio.h"
#include <stdint.h>
static uint32_t lapic_addr;

void lapic_write(uint16_t offset, uint32_t value)
{
  *(volatile uint32_t*)((uint64_t)lapic_addr + offset) = value;
}

uint32_t lapic_read(uint16_t offset)
{
  return *(volatile uint32_t*)((uint64_t)lapic_addr + offset);
}
void lapic_init()
{
  lapic_addr = madt_get_lapic_addr();
}

void lapic_enable()
{
  uint32_t lapic_low  = 0;
  uint32_t lapic_high = 0;

  read_msr(0x1B, &lapic_low, &lapic_high);
  write_msr(0x1B, (lapic_low | 0x800) & ~(0x100), lapic_high);

  lapic_write(0xF0, lapic_read(0xF0) | 0x100);
}

void lapic_disable()
{
  lapic_write(0xF0, lapic_read(0xF0) & ~(0x100));
}

void lapic_send_eoi()
{
  lapic_write(0xB0, 0);
}
