#include "acpi.h"
#include "acpi/tables.h"
#include <stdio.h>

void get_definition_block_count()
{
  acpi_fadt_t* fadt  = (acpi_fadt_t*)acpi_get_table("FACP");
  void* dsdt_address = (void*)fadt->x_dsdt;
  if (fadt->x_dsdt == 0)
  {
    dsdt_address = (void*)fadt->dsdt;
  }

  acpi_dsdt_t* dsdt = (acpi_dsdt_t*)dsdt_address;

  int length = (dsdt->header.length - sizeof(acpi_sdt_header_t));

  printf("Number of definition blocks: %d\n", length);
}
