#include "acpi.h"
void hpet_init()
{
  void* hpet_address = laihost_scan("HPET", 0);
  if (hpet_address == NULL)
  {
    return;
  }
  acpi_hpet_t* hpet = hpet_address;
}
