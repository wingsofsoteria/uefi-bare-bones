#include <types.h>
#include <qemu.h>
#include <stdbool.h>
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

bool sdt_checksum(acpi_sdt_header_t* sdt)
{
  debug("%.4s\n%d\n", sdt->signature, sdt->length);
  uint8_t* bytes = (uint8_t*)sdt;
  uint8_t sum    = 0;
  for (int i = 0; i < sdt->length; i++)
  {
    sum += bytes[i];
  }
  return sum == 0;
}

void xsdt(uint64_t address)
{
  acpi_xsdt_t* xsdt_description = (void*)VIRTUAL(address);
  if (!sdt_checksum(&xsdt_description->header))
  {
    debug_empty("Failed to enable ACPI\n");
  }

  uint64_t entry_count = (xsdt_description->header.length - 36) / 8;

  for (int i = 0; i < entry_count; i++)
  {
    acpi_sdt_header_t* entry_header = (void*)VIRTUAL(xsdt_description->entry[i]);
    if (!sdt_checksum(entry_header))
    {
      debug_empty("Failed to enable ACPI\n");
    }
  }
  debug_empty("ACPI Finished\n");
}

void rsdt(uint32_t address)
{
}
