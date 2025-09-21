#include "ioapic.h"
#include "madt.h"
#include "stdlib.h"

#include <stdint.h>

// TODO better helper functions / properly configure each IO APIC
// TODO store addresses of other IO APICs and configure them separately (right now it unmasks IOREDTBL1 on EVERY IO)

void set_redtbl(uint64_t ioregsel, ioapic_redtbl_t redtbl)
{
  write_ioapic(ioregsel, redtbl.pin, redtbl.trigger_mode << 15 | redtbl.pin_polarity << 13 | redtbl.destination_mode << 11 | redtbl.delivery_mode << 8 | redtbl.vector);
  write_ioapic(ioregsel, redtbl.pin + 1, (uint64_t)redtbl.destination << 56);
}

void enable_ps2_keyboard()
{
  uint32_t ioregsel                          = madt_get_ioapic(0);
  madt_interrupt_source_override_t* override = madt_get_override_for_irq(1);
  ioapic_redtbl_t redtbl;
  redtbl.vector           = 33;
  redtbl.delivery_mode    = 0;
  redtbl.destination_mode = 0;
  redtbl.mask             = 0;
  redtbl.destination      = 0;
  if (override != NULL)
  {
    redtbl.pin_polarity = override->polarity;
    redtbl.trigger_mode = override->trigger_mode;
    redtbl.pin          = override->global_system_interrupt + 0x10;
  }
  else
  {
    redtbl.pin_polarity = 0;
    redtbl.trigger_mode = 0;
    redtbl.pin          = 0x12;
  }
  set_redtbl(ioregsel, redtbl);
}

void write_ioapic(uint64_t ioregsel, const uint8_t offset, const uint32_t value)
{
  *(volatile uint32_t*)(ioregsel)        = offset;
  *(volatile uint32_t*)(ioregsel + 0x10) = value;
}

uint32_t read_ioapic(uint64_t ioregsel, const uint8_t offset)
{
  *(volatile uint32_t*)(ioregsel) = offset;
  return *(volatile uint32_t*)(ioregsel + 0x10);
}
// 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
