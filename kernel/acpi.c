#include "acpi.h"
#include "types.h"
#include <stdbool.h>
#include <stdio.h>

bool sdt_checksum(acpi_sdt_header_t* sdt)
{
  printf("%.4s\n%d\n", sdt->signature, sdt->length);
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
    printf("Failed to enable ACPI\n");
  }

  uint64_t entry_count = (xsdt_description->header.length - 36) / 8;

  for (int i = 0; i < entry_count; i++)
  {
    acpi_sdt_header_t* entry_header = (void*)VIRTUAL(xsdt_description->entry[i]);
    if (!sdt_checksum(entry_header))
    {
      printf("Failed to enable ACPI\n");
    }
  }
  printf("ACPI Finished\n");
}

void rsdt(uint32_t address)
{
}
