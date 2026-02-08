#ifndef __KERNEL_ACPI_INTERNAL_H__
#define __KERNEL_ACPI_INTERNAL_H__

#include "acpi/acpi.h"
#include "acpi/tables.h"

#define MADT_ADDR32(index)                                           \
  (madt->interrupt_controller_structure[index + 3] & 0xFF) << 24 |   \
    (madt->interrupt_controller_structure[index + 2] & 0xFF) << 16 | \
    (madt->interrupt_controller_structure[index + 1] & 0xFF) << 8 |  \
    madt->interrupt_controller_structure[index] & 0xFF;
#define MADT_LOOP                                                 \
  for (uint64_t i  = 0; i < madt->header.length - 44;             \
    i             += madt->interrupt_controller_structure[i + 1])

void madt_init();
void lapic_init();

uint32_t madt_get_ioapic(uint32_t gsi);
uint32_t madt_get_lapic_addr();
madt_interrupt_source_override_t* madt_get_override_for_irq(uint8_t irq);
acpi_sdt_header_t* acpi_get_table(char id[4]);
uint32_t lapic_read(uint16_t offset);
void lapic_write(uint16_t offset, uint32_t value);
void get_definition_block_count();
#endif
