// clang-format Language: C
#ifndef __KERNEL_ACPI_MADT_H__
#define __KERNEL_ACPI_MADT_H__

#include "acpi/acpi.h"
#include <stdint.h>

typedef struct
{
  acpi_sdt_header_t header;
  uint32_t local_interrupt_controller_address;
  uint32_t flags;
  char interrupt_controller_structure[];
} __attribute__((packed)) acpi_madt_t;

typedef struct
{
  uint8_t type;
  uint8_t length;
} __attribute__((packed)) madt_interrupt_controller_header_t;

typedef struct
{
  madt_interrupt_controller_header_t header;
  uint8_t io_apic_id;
  uint8_t reserved;
  uint32_t address;
  uint32_t global_system_interrupt_base;
} __attribute__((packed)) madt_io_apic_t;

typedef struct
{
  madt_interrupt_controller_header_t header;
  uint8_t bus;
  uint8_t source;
  uint32_t global_system_interrupt;
  uint8_t polarity : 2;
  uint8_t trigger_mode : 2;
  uint16_t reserved : 12;
} __attribute__((packed)) madt_interrupt_source_override_t;
void set_madt(uint64_t address);
uint32_t madt_get_ioapic(uint32_t gsi);
madt_interrupt_source_override_t* madt_get_override_for_irq(uint8_t irq);
#endif
