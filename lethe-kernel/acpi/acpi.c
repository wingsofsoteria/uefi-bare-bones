// SSDT and DSDT require an AML parser which I am putting off as long as
// possible BGRT is purely for cosmetic reasons

#include "acpi.h"

#include "aml.h"
#include "log.h"
#include "stdio.h"
#include "stdlib.h"
#include "types.h"

#include <string.h>
#include <utils.h>
static acpi_xsdt_t* XSDT = NULL;

static bool sdt_checksum(acpi_header_t* sdt)
{
  uint8_t* bytes = (uint8_t*)sdt;
  uint8_t  sum   = 0;
  for (int i = 0; i < sdt->length; i++) { sum += bytes[i]; }
  return sum == 0;
}

void acpi_dump_tables()
{
  int entries = (XSDT->header.length - sizeof(acpi_header_t)) / 8;
  for (int i = 0; i < entries; i++)
    {
      acpi_header_t* sdt = (void*)(XSDT->tables[i] + hhdm_mapping);
      klog("ACPI Table %.4s\n", sdt->signature);
    }
}

void* scan_tables(const char* sig, size_t index)
{
  int current_index = 0;
  if (strncmp(sig, "DSDT", 4) == 0)
    {
      acpi_fadt_t* fadt = scan_tables("FACP", 0);
      return (void*)(fadt->x_dsdt + hhdm_mapping);
    }
  int entries = (XSDT->header.length - sizeof(acpi_header_t)) / 8;
  for (int i = 0; i < entries; i++)
    {
      acpi_header_t* sdt = (void*)(XSDT->tables[i] + hhdm_mapping);
      klog("ACPI Table %.4s\n", sdt->signature);
      if (strncmp(sig, sdt->signature, 4) == 0)
        {
          if (current_index != index)
            {
              current_index++;
              continue;
            }
          return sdt;
        }
    }
  klog("Could not find table with signature %s\n", sig);
  return NULL;
}

int acpi_early_init(void* rsdp_pointer)
{
  acpi_xsdp_t* rsdp = rsdp_pointer;

  XSDT = (void*)(rsdp->xsdt + hhdm_mapping);
  if (!sdt_checksum(&XSDT->header)) { panic("Failed to parse ACPI tables\n"); }
  madt_init();
  lapic_init();
  return 0;
}

int acpi_late_init()
{
  parse_table(scan_tables("DSDT", 0));
  klog("ACPI initialization finished\n");
  return 0;
}

// void shutdown() { lai_enter_sleep(5); }
