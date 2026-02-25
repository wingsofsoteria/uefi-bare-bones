#ifndef __KERNEL_ACPI_TABLES_H__
#define __KERNEL_ACPI_TABLES_H__

#include <stdint.h>
typedef struct
{
  char signature[4];
  uint32_t length;
  uint8_t revision;
  uint8_t checksum;
  uint8_t oem_id[6];
  uint64_t oem_table_id;
  uint32_t oem_revision;
  uint32_t creator_id;
  uint32_t creator_revision;
} __attribute__((packed)) acpi_sdt_header_t;

typedef struct
{
  acpi_sdt_header_t header;
  uint64_t entry[];
} __attribute__((packed)) acpi_xsdt_t;

// TODO i'm saving my fingers from manually typing out all members of the FADT
// table by just shoving it all in arrays. I'll add them as needed

typedef struct
{
  acpi_sdt_header_t header;
  uint32_t firmware_ctrl;
  uint32_t dsdt;
  uint8_t __unused_block_1[96];
  uint64_t x_dsdt;
  uint8_t __unused_block_2[120];
} __attribute__((packed)) acpi_fadt_t;

typedef struct
{
  acpi_sdt_header_t header;
  uint8_t definition_blocks[];
} __attribute__((packed)) acpi_aml_table_t;

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

#endif
