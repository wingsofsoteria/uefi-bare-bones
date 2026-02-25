#include "acpi.h"
#include "aml/aml.h"
void dsdt_parse()
{
  acpi_fadt_t* fadt  = (acpi_fadt_t*)acpi_get_table("FACP");
  void* dsdt_address = (void*)fadt->x_dsdt;
  if (fadt->x_dsdt == 0)
  {
    dsdt_address = (void*)fadt->dsdt;
  }
  aml_parser_init(dsdt_address);
  aml_parser_run();
}
