#include "acpi/mcfg.h"

#include "acpi.h"
#include "acpi/acpi.h"
#include "config.h"
#include "log.h"
#include "pci/pci.h"
#include "types.h"

#include <stdint.h>

static acpi_mcfg_t* MCFG = NULL;

void init_mcfg()
{
  void* mcfg_ptr = scan_tables("MCFG", 0);
  if (!mcfg_ptr) return;

  kernel_config.has_mcfg = 1;
  MCFG                   = (acpi_mcfg_t*)mcfg_ptr;
}

uint64_t get_mmio_ecam_addr(uint8_t bus)
{
  static uint64_t entries = UINT64_MAX;
  if (entries == UINT64_MAX)
    {
      uint64_t config_space_length = MCFG->header.length - 44;
      entries                      = config_space_length / 16;
    }

  for (int i = 0; i < entries; i++)
    {
      struct mmio_configuration_space cfg = MCFG->config_space[i];
      if (bus >= cfg.bus_start && bus <= cfg.bus_end)
        {
          return cfg.base_address;
        }
    }

  return UINT64_MAX;
}

