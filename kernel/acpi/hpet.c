#include "acpi.h"
#include "acpi/tables.h"
#include "stdlib.h"
void hpet_init()
{
  void* hpet_address = acpi_get_table("HPET");
  if (hpet_address == NULL)
  {
    return;
  }
  acpi_hpet_t* hpet = hpet_address;
}
