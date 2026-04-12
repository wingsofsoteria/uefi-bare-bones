#pragma once
#include "acpispec/tables.h"
#include <stdint.h>

typedef struct
{
  acpi_header_t header;
  uint8_t hardware_rev_id;
  uint8_t comparator_count : 5;
  uint8_t counter_size : 1;
  uint8_t reserved : 1;
  uint8_t legacy_replacement : 1;
  uint16_t pci_vendor_id;

  uint8_t address_space_id;
  uint8_t register_bit_width;
  uint8_t register_bit_offset;
  uint8_t _reserved;
  uint64_t address;

  uint8_t hpet_number;
  uint16_t min_clock_tick;
  uint8_t page_protection;
} __attribute__((packed)) acpi_hpet_t;

// TODO i'm saving my fingers from manually typing out all members of the FADT
// table by just shoving it all in arrays. I'll add them as needed

typedef struct
{
  acpi_header_t header;
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
