#include "acpi.h"
#include "stdlib.h"
#include "types.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

bool sdt_checksum(acpi_sdt_header_t* sdt)
{
  uint8_t* bytes = (uint8_t*)sdt;
  uint8_t sum    = 0;
  for (int i = 0; i < sdt->length; i++)
  {
    sum += bytes[i];
  }
  return sum == 0;
}

void madt(uint64_t address)
{
}

void setup_acpi(uint64_t xsdt_address)
{
  acpi_xsdt_t* description_table = (void*)VIRTUAL(xsdt_address);
  if (!sdt_checksum(&description_table->header))
  {
    printf("Failed to parse ACPI tables\n");
  }

  uint64_t entry_count = (description_table->header.length - 36) / 8;

  for (int i = 0; i < entry_count; i++)
  {
    acpi_sdt_header_t* header = (void*)VIRTUAL(description_table->entry[i]);
    if (!sdt_checksum(header))
    {
      printf("Invalid ACPI Table\n");
      return;
    }

    if (strncmp(header->signature, "APIC", 4) == 0)
    {
      madt(description_table->entry[i]);
    }
  }
  printf("Finished parsing ACPI tables\n");
}
