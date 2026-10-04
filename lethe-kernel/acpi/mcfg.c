#include "acpi/mcfg.h"

#include "acpi.h"
#include "acpi/acpi.h"
#include "config.h"
#include "memory/alloc.h"
#include "pci/pci.h"
#include "stddef.h"
#include "utils.h"

#include <stdint.h>

struct mcfg_entry
{
  uint64_t           base_address;
  uint16_t           group_num;
  uint8_t            bus_start;
  uint8_t            bus_end;
  struct mcfg_entry* next;
};

static acpi_mcfg_t*       MCFG       = NULL;
static struct mcfg_entry* mcfg_start = NULL;

static struct mcfg_entry* new_entry(struct mmio_configuration_space mcfg_entry)
{
  struct mcfg_entry* entry = kmalloc(sizeof(struct mcfg_entry));

  entry->bus_start    = mcfg_entry.bus_start;
  entry->bus_end      = mcfg_entry.bus_end;
  entry->group_num    = mcfg_entry.group_num;
  entry->base_address = mcfg_entry.base_address;
  entry->next         = NULL;
  return entry;
}

void parse_mcfg()
{
  if (!MCFG) return;
  if (mcfg_start) return;
  mcfg_start                   = new_entry(MCFG->config_space[0]);
  uint64_t config_space_length = MCFG->header.length - 44;
  uint64_t entries             = config_space_length / 16;

  for (int i = 1; i < entries; i++)
    {
      struct mcfg_entry* entry = new_entry(MCFG->config_space[i]);
      entry->next              = mcfg_start->next;
      mcfg_start->next         = entry;
    }
}

void init_mcfg()
{
  void* mcfg_ptr = scan_tables("MCFG", 0);
  if (!mcfg_ptr) return;

  kernel_config.has_mcfg = 1;
  MCFG                   = (acpi_mcfg_t*)mcfg_ptr;
  parse_mcfg();
}

void* get_mmio_ecam_addr(uint16_t group, uint8_t bus)
{
  panic("dont use this yet");
  return NULL;
  struct mcfg_entry* entry = mcfg_start;
  while (entry)
    {

      if (
        group == entry->group_num && bus >= entry->bus_start &&
        bus <= entry->bus_end
      )
        {
        }
      entry = entry->next;
    }
  return NULL;
}

