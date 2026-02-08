// SSDT and DSDT require an AML parser which I am putting off as long as
// possible BGRT is purely for cosmetic reasons

#include "acpi.h"
#include "stdlib.h"
#include "types.h"
#include <stdio.h>
static acpi_xsdt_t* XSDT = NULL;

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

acpi_sdt_header_t* acpi_get_table(char id[4])
{
  int entries = (XSDT->header.length - sizeof(acpi_sdt_header_t)) / 8;
  for (int i = 0; i < entries; i++)
  {
    acpi_sdt_header_t* sdt = (void*)XSDT->entry[i];
    if (id[0] != sdt->signature[0] || id[1] != sdt->signature[1] ||
      id[2] != sdt->signature[2] || id[3] != sdt->signature[3])
    {
      continue;
    }
    return sdt;
  }

  return NULL;
}

void acpi_init(uint64_t xsdt_address)
{
  XSDT = (void*)VIRTUAL(xsdt_address);
  if (!sdt_checksum(&XSDT->header))
  {
    printf("Failed to parse ACPI tables\n");
    abort();
  }
  madt_init();
  lapic_init();
  // TODO finish parsing FADT
}
