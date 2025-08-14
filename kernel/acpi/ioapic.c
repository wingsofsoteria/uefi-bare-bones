#include <stdint.h>
#include <stdio.h>
static uint64_t ioregsel;
void write_ioapic(const uint8_t offset, const uint32_t value);
uint32_t read_ioapic(const uint8_t offset);

void unmask_redtbl(uint8_t base)
{
  uint32_t redtbl00  = read_ioapic(base);
  uint32_t redtbl01  = read_ioapic(base + 1);
  redtbl00          &= ~(1 << 16);
  write_ioapic(base, redtbl00);
}

void program_ioapic(uint32_t p_addr, uint32_t gsi_base)
{
  printf("IO APIC ADDR: %x, GSI: %d\n", p_addr, gsi_base);
  ioregsel            = p_addr;
  uint32_t io_apic_id = read_ioapic(0);
  unmask_redtbl(0x10);
  printf("IO APIC ID: %b\n", io_apic_id);
}

void write_ioapic(const uint8_t offset, const uint32_t value)
{
  *(volatile uint32_t*)(ioregsel)        = offset;
  *(volatile uint32_t*)(ioregsel + 0x10) = value;
}

uint32_t read_ioapic(const uint8_t offset)
{
  *(volatile uint32_t*)(ioregsel) = offset;
  return *(volatile uint32_t*)(ioregsel + 0x10);
}
// 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
