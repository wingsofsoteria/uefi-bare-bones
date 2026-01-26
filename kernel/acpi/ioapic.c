#include "acpi/acpi.h"
#include "acpi/madt.h"
#include "acpi/ioapic.h"

#include <stdlib.h>
#include <stdint.h>

// TODO better helper functions / properly configure each IO APIC
// TODO get correct ioapic instead of defaulting to 0 (and figure out if qemu can emulate more than one ioapic)

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

void set_redirection_table_entry(uint64_t ioregsel, ioapic_redtbl_t redtbl)
{
  write_ioapic(ioregsel, redtbl.pin, redtbl.trigger_mode << 15 | redtbl.pin_polarity << 13 | redtbl.destination_mode << 11 | redtbl.delivery_mode << 8 | redtbl.vector);
  write_ioapic(ioregsel, redtbl.pin + 1, (uint64_t)redtbl.destination << 56);
}

void disable_irq(int irq)
{
  uint32_t register_selector                            = madt_get_ioapic(0);
  madt_interrupt_source_override_t* interrupt_overrides = madt_get_override_for_irq(irq);
  uint32_t pin                                          = 0x10 + (irq * 2);
  if (interrupt_overrides != NULL)
  {
    pin = 0x10 + (interrupt_overrides->global_system_interrupt * 2);
  }
  write_ioapic(register_selector, pin, 1 << 16); // set interrupt mask bit
}

void enable_irq(int irq, int vector)
{
  uint32_t register_selector                            = madt_get_ioapic(0);
  madt_interrupt_source_override_t* interrupt_overrides = madt_get_override_for_irq(irq);
  ioapic_redtbl_t redirection_table;

  redirection_table.vector           = vector;
  redirection_table.delivery_mode    = 0;
  redirection_table.destination_mode = 0;
  redirection_table.mask             = 0;
  redirection_table.destination      = 0;
  if (interrupt_overrides != NULL)
  {
    redirection_table.pin_polarity = interrupt_overrides->polarity;
    redirection_table.trigger_mode = interrupt_overrides->trigger_mode;
    redirection_table.pin          = 0x10 + (interrupt_overrides->global_system_interrupt * 2);
  }
  else
  {
    redirection_table.pin_polarity = 0;
    redirection_table.trigger_mode = 0;
    redirection_table.pin          = 0x10 + (irq * 2);
  }
  set_redirection_table_entry(register_selector, redirection_table);
}
