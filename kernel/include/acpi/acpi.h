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
} __attribute__((packed)) acpi_local_apic_structure_t;

typedef struct
{
  uint8_t type;
  uint8_t length;
  uint8_t io_apic_id;
  uint8_t reserved;
  uint32_t io_apic_addr;
  uint32_t global_system_interrupt_base;
} __attribute__((packed)) acpi_io_apic_structure_t;

typedef struct
{
  acpi_sdt_header_t header;
  uint64_t entry[];
} __attribute__((packed)) acpi_xsdt_t;

void acpi_init(uint64_t);
void madt_init();
void lapic_init();
acpi_sdt_header_t* acpi_get_table(char id[4]);
void enable_irq(int irq, int vector);
void disable_irq(int irq);
#endif
