#include "pci/ahci.h"

#include "cpu/idt.h"
#include "log.h"
#include "memory/alloc.h"
#include "memory/paging.h"
#include "pci/pci.h"
#include "types.h"
#include "utils.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static ahci_dev_t* device_list = NULL;

static int find_cmdslot(ahci_port_t* port)
{
  uint32_t slots = (port->sact | port->ci);
  for (int i = 0; i < 32; i++)
    {
      if (!(slots & 1)) { return i; }
      slots >>= 1;
    }

  return -1;
}

static ahci_cmd_tbl_t* get_cmd_table(ahci_cmd_hdr_t* header)
{
  uint64_t physical = (header->ctba | (uint64_t)header->ctbau << 32);
  uint64_t virtual  = physical + hhdm_mapping;
  if (!IS_PAGE_ALIGNED(virtual)) { abort(); }
  if (virtual_to_physical(virtual) != physical)
    {
      map_page(
        virtual,
        physical,
        PAGE_PRESENT | PAGE_WRITABLE | PAGE_CACHE_DISABLE
      );
    }
  return (void*)virtual;
}

static void stop_cmd(ahci_port_t* port)
{

  port->cmd &= ~(1 << 0);
  port->cmd &= ~(1 << 4);
  while (true)
    {
      if (port->cmd & 1 << 14) { continue; }
      if (port->cmd & 1 << 15) { continue; }
      break;
    }
}

static void start_cmd(ahci_port_t* port)
{
  while (port->cmd & 1 << 15);
  port->cmd |= 1 << 4;
  port->cmd |= 1 << 0;
}

static ahci_cmd_t* identify(ahci_dev_t* device)
{
  ahci_cmd_hdr_t* header = device->command_header;

  int slot = find_cmdslot(device->port);
  if (slot == -1) { return false; }

  ahci_cmd_t* command = kmalloc(sizeof(ahci_cmd_t));
  command->port       = device->port_number;
  command->slot       = slot;
  command->data       = kmalloc(sizeof(char) * 512);
  command->d_len      = 512;
  command->complete   = false;
  command->next       = device->command_list;
  if (device->command_list) { device->command_list->prev = command; }
  device->command_list = command;

  header               += slot;
  header->cfl           = sizeof(ahci_fis_h2d_t) / sizeof(uint32_t);
  header->w             = 0;
  header->prdtl         = 1;
  ahci_cmd_tbl_t* table = get_cmd_table(header);
  memset(table, 0, sizeof(ahci_cmd_tbl_t));
  uint64_t paddr            = virtual_to_physical((uint64_t)command->data);
  table->prdt_entry[0].dba  = paddr & 0xFFFFFFFF;
  table->prdt_entry[0].dbau = paddr >> 32;
  table->prdt_entry[0].dbc  = 511;
  table->prdt_entry[0].i    = 1;

  ahci_fis_h2d_t* fis = (ahci_fis_h2d_t*)(&table->cfis);
  fis->fis_type       = FIS_TYPE_REG_H2D;
  fis->c              = 1;
  fis->cmd            = 0xEC;
  fis->dev            = device->port_number;

  device->port->ci = 1 << slot;
  return command;
}

static void init_sata_drive(ahci_dev_t* device)
{
  klog("SATA device on port: %d\n", device->port_number);
  ahci_cmd_t* command = identify(device);
  while (!command->complete);
  klog("command is complete, read data\n");
  uint16_t* buffer = command->data;
  klog(
    "%llx",
    (uint64_t)buffer[100] | (uint64_t)buffer[101] << 16 |
      (uint64_t)buffer[102] << 32 | (uint64_t)buffer[103] << 48
  );

  kfree(command);
}

static void srst(ahci_dev_t* device)
{
  stop_cmd(device->port);
  start_cmd(device->port);
  assert((device->port->cmd & 0b10001000) == 0);
  ahci_cmd_hdr_t* header = device->command_header;
  header->cfl            = sizeof(ahci_fis_h2d_t) / sizeof(uint32_t);
  header->w              = 0;
  header->prdtl          = 0;
  header->c              = 1;
  header->r              = 1;
  ahci_cmd_tbl_t* table  = get_cmd_table(header);
  memset(table, 0, sizeof(ahci_cmd_tbl_t));
  ahci_fis_h2d_t* fis = (ahci_fis_h2d_t*)&table->cfis;
  fis->c              = 0;
  fis->ctl            = 1 << 2;

  header       += 1;
  header->cfl   = sizeof(ahci_fis_h2d_t) / sizeof(uint32_t);
  header->w     = 0;
  header->prdtl = 0;
  header->c     = 0;
  header->r     = 0;
  table         = get_cmd_table(header);
  memset(table, 0, sizeof(ahci_cmd_tbl_t));
  fis      = (ahci_fis_h2d_t*)&table->cfis;
  fis->c   = 0;
  fis->ctl = 0;
  while (device->port->tfd & (1 << 7));
  device->port->is = (uint32_t)-1;
  device->port->ie = (uint32_t)-1;
}

static void init_port(ahci_port_t* port, int port_number)
{
  if ((port->ssts & 0b111) != 0x3) return;
  uint64_t clb = port->clb | (uint64_t)port->clbu << 32;
  uint64_t fb  = port->fb | (uint64_t)port->fbu << 32;
  if (!IS_PAGE_ALIGNED(clb)) { clb = PAGE_ALIGN(clb); }
  if (!IS_PAGE_ALIGNED(fb)) { fb = PAGE_ALIGN(fb); }
  klog("%llx, %llx\n", clb, fb);
  map_page(
    clb + hhdm_mapping,
    clb,
    PAGE_PRESENT | PAGE_WRITABLE | PAGE_CACHE_DISABLE
  );
  map_page(
    fb + hhdm_mapping,
    fb,
    PAGE_PRESENT | PAGE_WRITABLE | PAGE_CACHE_DISABLE
  );
  ahci_dev_t* device     = kmalloc(sizeof(ahci_dev_t));
  device->type           = port->sig;
  device->port_number    = port_number;
  device->port           = port;
  device->next           = device_list;
  device->fis_base       = (void*)(fb + hhdm_mapping);
  device->command_header = (void*)(clb + hhdm_mapping);
  device->command_list   = NULL;
  device_list            = device;
  switch (device->type)
    {
      case SATA_SIG_ATA:
        {
          srst(device);
          init_sata_drive(device);
          break;
        }
      default:
        {
          klog("unimplemented sig: %x\n", port->sig);
          break;
        }
    }
}

static void enumerate_ahci_devices(ahci_mem_t* abar)
{
  uint32_t pi = abar->ghc.pi; // bitmask of implemented ports
  klog("enumerating %p\n", abar);
  for (int i = 0; i < 32; i++)
    {
      int port_implemented = pi & (1 << i);
      if (!port_implemented) continue;
      init_port(&abar->ports[i], i);
    }
}

static void enumerate_ahci_controllers(int max)
{
  for (int i = 0; i < max; i++) {}
}

extern int check_msi_capability(void*);

extern int enable_msi(
  void*     device_handle,
  int       msi_cap_offset,
  interrupt handler
);

static ahci_mem_t* abar = NULL;

static ahci_dev_t* get_device(int port)
{
  ahci_dev_t* device = device_list;
  while (device)
    {
      if (device->port_number == port) { return device; }
      device = device->next;
    }
  return NULL;
}

static isr_stack_t* ahci_irq(isr_stack_t* stack)
{
  if (!abar) { panic("ABAR uninitialized"); }
  uint32_t is = abar->ghc.is;
  for (int port = 0; port < 32; port++)
    {
      uint32_t port_ips = is & (1 << port);
      if (!port_ips) { continue; }
      klog("Port %d interrupt\n", port);
      ahci_dev_t* device = get_device(port);
      if (!device) { panic("fatal error occurred"); }
      ahci_cmd_t* command_list = device->command_list;
      uint8_t     port_ci      = device->port->ci;
      uint8_t     port_is      = device->port->is;
      if (port_ci && !command_list) { panic("Unregistered command issued\n"); }
      if (!port_is) { continue; }
      // apparently you need to set the bit you want to clear?
      device->port->is = (uint32_t)-1;
      abar->ghc.is     = 1 << port;
      while (command_list)
        {
          if ((port_ci & (1 << command_list->slot)) == 0)
            {
              command_list->complete = true;
              if (!command_list->prev)
                {
                  device->command_list = device->command_list->next;
                }
              else
                {
                  command_list->prev->next = command_list->next;
                }
            }
          command_list = command_list->next;
        }
    }
  return stack;
}

void init_ahci()
{
  void* controller_handle = locate_pci_device(0x1, 0x6, 0);
  if (!controller_handle) return;
  if (!pci_compare_value(controller_handle, 3, 0, 0x007F0000))
    {
      klog(
        "got header type: 0x%x instead of 0x0\n",
        get_register_value(controller_handle, 3)
      );
      return;
    }
  int msi_cap = check_msi_capability(controller_handle);

  if (msi_cap) { enable_msi(controller_handle, msi_cap, ahci_irq); }
  uint32_t phys_addr = get_register_value(controller_handle, 0x9);
  abar               = (void*)map_page_nearest(
    phys_addr + hhdm_mapping,
    phys_addr,
    PAGE_PRESENT | PAGE_WRITABLE | PAGE_CACHE_DISABLE
  );
  abar->ghc.is   = (uint32_t)-1;
  abar->ghc.ghc |= 1 << 31;
  abar->ghc.ghc |= 1 << 1;

  klog("GHC: %b\n", abar->ghc.ghc);
  enumerate_ahci_devices(abar);
  klog(
    "%x %x\n",
    get_register_value(controller_handle, (msi_cap / 4) + 0x14),
    get_register_value(controller_handle, (msi_cap / 4) + 0x10)
  );
  klog(
    "debug: %zx %zx %zx %zx\n",
    sizeof(ahci_ghc_t),
    sizeof(ahci_cap_t),
    sizeof(ahci_port_t),
    sizeof(ahci_mem_t)
  );
}
