#include "acpi/acpi.h"
#include "graphics/pixel.h"
#include "madt.h"
#include "types.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// TODO write parsers for some other tables (HPET SSDT/DSDT* BGRT* etc)
// SSDT and DSDT require an AML parser which I am putting off as long as possible
// BGRT is purely for cosmetic reasons

bool sdt_checksum(acpi_sdt_header_t* sdt)
{
  printf("%.4s ", sdt->signature);
  uint8_t* bytes = (uint8_t*)sdt;
  uint8_t sum    = 0;
  for (int i = 0; i < sdt->length; i++)
  {
    sum += bytes[i];
  }
  return sum == 0;
}

void setup_acpi(uint64_t xsdt_address)
{
  clear_screen();
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
      set_madt(description_table->entry[i]);
    }
  }
  printf("Finished parsing ACPI tables\n");
}
