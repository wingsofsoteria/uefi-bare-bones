#include "acpi.h"

#include <stdint.h>

// TODO properly configure LAPIC

static uint32_t lapic_addr;

void lapic_write(uint16_t offset, uint32_t value)
{
  *(volatile uint32_t*)((uint64_t)lapic_addr + offset) = value;
}

uint32_t lapic_read(uint16_t offset)
{
  return *(volatile uint32_t*)((uint64_t)lapic_addr + offset);
}

void read_msr(uint32_t msr, uint32_t* low, uint32_t* high)
{
  asm volatile("rdmsr"
    : "=a"(*low), "=d"(*high)
    : "c"(msr));
}

void write_msr(uint32_t msr, uint32_t low, uint32_t high)
{
  asm volatile("wrmsr"
    :
    : "a"(low), "d"(high), "c"(msr));
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
  // lapic_write(0x320, lapic_read(0x320) & ~(0x10000));
  asm volatile("sti");
}

void lapic_disable()
{
  asm volatile("cli");
  lapic_write(0xF0, lapic_read(0xF0) & ~(0x100));
}

void lapic_send_eoi()
{
  lapic_write(0xB0, 0);
}