// clang-format Language: C
#ifndef __KERNEL_ACPI_H__
#define __KERNEL_ACPI_H__

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
  uint8_t type;
  uint8_t length;
  uint8_t processor_uid;
  uint8_t apic_id;
  uint32_t flags;
} acpi_local_apic_structure_t;

typedef struct
{
  uint8_t type;
  uint8_t length;
  uint8_t io_apic_id;
  uint8_t reserved;
  uint32_t io_apic_addr;
  uint32_t global_system_interrupt_base;
} acpi_io_apic_structure_t;

// typedef struct
// {
//   uint8_t type;
//   uint8_t length;
//   union
//   {
//     acpi_local_apic_structure_t local_apic_structure;
//     acpi_io_apic_structure_t io_apic_structure;
//   };
// } __attribute__((packed)) acpi_interrupt_controller_structure_t;

typedef struct
{
  acpi_sdt_header_t header;
  uint32_t local_interrupt_controller_address;
  uint32_t flags;
  // acpi_interrupt_controller_structure_t* interrupt_controller_structure[];
  char interrupt_controller_structure[];
} __attribute__((packed)) acpi_madt_t;

typedef struct
{
  acpi_sdt_header_t header;
  uint64_t entry[];
} __attribute__((packed)) acpi_xsdt_t;

void setup_acpi(uint64_t);

#endif
