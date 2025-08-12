#ifndef __KERNEL_ACPI_H__
#define __KERNEL_ACPI_H__

#include <stdint.h>
typedef struct
{
  uint8_t signature[4];
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

void xsdt(uint64_t addr);

#endif
