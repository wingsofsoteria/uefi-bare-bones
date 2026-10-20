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

#define SATA_SIG_ATA       0x00000101 // SATA drive
#define SATA_SIG_ATAPI     0xEB140101 // SATAPI drive
#define SATA_SIG_SEMB      0xC33C0101 // Enclosure management bridge
#define SATA_SIG_PM        0x96690101 // Port multiplier
#define FIS_TYPE_REG_H2D   0x27
#define FIS_TYPE_REG_D2H   0x34
#define FIS_TYPE_DMA_ACT   0x39
#define FIS_TYPE_DMA_SETUP 0x41
#define FIS_TYPE_DATA      0x46
#define FIS_TYPE_BIST      0x58
#define FIS_TYPE_PIO_SETUP 0x5F
#define FIS_TYPE_DEV_BITS  0xA1

typedef volatile struct ahci_host_capabilities_layout
{
  uint8_t np : 5;   // number of ports
  uint8_t sxs : 1;  // supports external SATA
  uint8_t ems : 1;  // supports enclosure management
  uint8_t cccs : 1; // supports command completion coalescing
  uint8_t ncs : 4;  // number of command slots
  uint8_t psc : 1;  // supports transitions to the partial state
  uint8_t ssc : 1;  // supports transitions to the slumber state
  uint8_t pmd : 1;  // supports multiple DRQ block transfer through PIO
  uint8_t fbss : 1; // supports FIS switching
  uint8_t spm : 1;  // supports port multiplier
  uint8_t sam : 1;  // supports AHCI mode only
  uint8_t reserved : 1;
  uint8_t iss : 4;   // max interface speed
  uint8_t sclo : 1;  // supports command list override
  uint8_t sal : 1;   // supports LED activity
  uint8_t salp : 1;  // supports aggressive link power management
  uint8_t sss : 1;   // supports staggered spin-up
  uint8_t smps : 1;  // supports mechanical presence switch
  uint8_t ssntf : 1; // supports SNotification register (HBA_PORT.SNTF)
  uint8_t sncq : 1;  // supports native command queuing
  uint8_t s64a : 1;  // supports 64-bit addressing
} ahci_cap_t;

typedef volatile struct ahci_generic_host_control_layout
{
  ahci_cap_t cap;       // host capabilities
  uint32_t   ghc;       // global host control
  uint32_t   is;        // interrupt status
  uint32_t   pi;        // ports implemented
  uint32_t   vs;        // version
  uint32_t   ccc_ctl;   // command completion coalescing control
  uint32_t   ccc_ports; // command completion coalescing ports
  uint32_t   em_loc;    // enclosure management location
  uint32_t   em_ctl;    // enclosure management control
  uint32_t   cap2;      // extended host capabilities
  uint32_t   bohc;      // bios/os handoff control and status
} ahci_ghc_t;

typedef volatile struct ahci_port_register_layout
{
  uint32_t clb;  // command list base address
  uint32_t clbu; // command list base address upper
  uint32_t fb;   // FIS base address
  uint32_t fbu;  // FIS base address upper
  uint32_t is;   // interrupt status
  uint32_t ie;   // interrupt enable
  uint32_t cmd;  // command and status
  uint32_t reserved;
  uint32_t tfd;    // task file data
  uint32_t sig;    // signature
  uint32_t ssts;   // serial ATA status
  uint32_t sctl;   // serial ATA control
  uint32_t serr;   // serial ATA error
  uint32_t sact;   // serial ATA active
  uint32_t ci;     // command issue
  uint32_t sntf;   // serial ATA notification
  uint32_t fbs;    // FIS based switching control
  uint32_t devslp; // device sleep
  uint8_t  reserved_2[0x28];
  uint8_t  vs[0xF]; // vendor specific
} ahci_port_t;

typedef volatile struct ahci_memory_register_layout
{
  ahci_ghc_t  ghc;
  uint8_t     reserved[0xA0 - 0x2C];
  uint8_t     vendor_specific[0x100 - 0xA0];
  ahci_port_t ports[1];
} ahci_mem_t;

typedef struct ahci_fis_register_host_to_device
{
  uint8_t fis_type;
  uint8_t pmport : 4;
  uint8_t rsv0 : 3;
  uint8_t c : 1;
  uint8_t cmd;
  uint8_t featurel;
  uint8_t lba0;
  uint8_t lba1;
  uint8_t lba2;
  uint8_t dev;
  uint8_t lba3;
  uint8_t lba4;
  uint8_t lba5;
  uint8_t featureh;
  uint8_t countl;
  uint8_t counth;
  uint8_t icc;
  uint8_t ctl;
  uint8_t rsv1[4];
} ahci_fis_h2d_t;

typedef struct ahci_fis_register_device_to_host
{
  uint8_t fis_type;
  uint8_t pmport : 4;
  uint8_t rsv0 : 2;
  uint8_t i : 1;
  uint8_t rsv1 : 1;
  uint8_t sts;
  uint8_t err;
  uint8_t lba0;
  uint8_t lba1;
  uint8_t lba2;
  uint8_t dev;
  uint8_t lba3;
  uint8_t lba4;
  uint8_t lba5;
  uint8_t rsv2;
  uint8_t countl;
  uint8_t counth;
  uint8_t rsv3[2];
  uint8_t rsv4[4];
} ahci_fis_d2h_t;

typedef struct ahci_fis_data_bi
{
  uint8_t  fis_type;
  uint8_t  pmport : 4;
  uint8_t  rsv0 : 4;
  uint8_t  rsv1[2];
  uint32_t data[1]; // variable size
} ahci_fis_data_t;

typedef struct ahci_fis_pio_setup_d2h
{
  uint8_t  fis_type;
  uint8_t  pmport : 4;
  uint8_t  rsv0 : 1;
  uint8_t  d : 1;
  uint8_t  i : 1;
  uint8_t  rsv1 : 1;
  uint8_t  sts;
  uint8_t  err;
  uint8_t  lba0;
  uint8_t  lba1;
  uint8_t  lba2;
  uint8_t  dev;
  uint8_t  lba3;
  uint8_t  lba4;
  uint8_t  lba5;
  uint8_t  rsv2;
  uint8_t  countl;
  uint8_t  counth;
  uint8_t  rsv3;
  uint8_t  e_status;
  uint16_t tc;
  uint8_t  rsv4[2];
} ahci_fis_pio_d2h_t;

typedef struct ahci_fis_dma_setup_d2h
{
  uint8_t  fis_type;
  uint8_t  pmport : 4;
  uint8_t  rsv0 : 1;
  uint8_t  d : 1;
  uint8_t  i : 1;
  uint8_t  a : 1;
  uint8_t  rsv1[2];
  uint64_t dma_buf_id;
  uint32_t rsv2;
  uint32_t dma_buf_offset;
  uint32_t tc;
  uint32_t rsv3;
} ahci_fis_dma_d2h_t;

typedef struct ahci_frame_information_structure
{
  ahci_fis_dma_d2h_t dsfis;
  uint8_t            pad0[4];
  ahci_fis_pio_d2h_t psfis;
  uint8_t            pad1[12];
  ahci_fis_d2h_t     rfis;
  uint8_t            pad2[4];
  uint8_t            sdbfis[2];
  uint8_t            ufis[64];
  uint8_t            rsv[0x100 - 0xA0];
} ahci_fis_t;

typedef struct ahci_command_header
{
  uint8_t           cfl : 5;
  uint8_t           a : 1;
  uint8_t           w : 1;
  uint8_t           p : 1;
  uint8_t           r : 1;
  uint8_t           b : 1;
  uint8_t           c : 1;
  uint8_t           rsv0 : 1;
  uint8_t           pmp : 4;
  uint16_t          prdtl;
  volatile uint32_t prdbc;
  uint32_t          ctba;
  uint32_t          ctbau;
  uint32_t          rsv1[4];
} ahci_cmd_hdr_t;

typedef struct ahci_prdt_entry
{
  uint32_t dba;
  uint32_t dbau;
  uint32_t rsv0;
  uint32_t dbc : 22;
  uint32_t rsv1 : 9;
  uint32_t i : 1;
} ahci_prdt_t;

typedef struct ahci_command_table
{
  uint8_t     cfis[64];
  uint8_t     acmd[16];
  uint8_t     rsv[48];
  ahci_prdt_t prdt_entry[1];
} ahci_cmd_tbl_t;

// WIP struct
typedef struct ahci_device
{
  uint32_t            type;
  uint8_t             port_number;
  ahci_port_t*        port;
  ahci_cmd_hdr_t*     command_header;
  void*               fis_base;
  struct ahci_device* next;
} ahci_dev_t;

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

static void stop_cmd(ahci_dev_t* device)
{

  device->port->cmd &= ~(1 << 0);
  device->port->cmd &= ~(1 << 4);
  while (true)
    {
      if (device->port->cmd & 1 << 14) { continue; }
      if (device->port->cmd & 1 << 15) { continue; }
      break;
    }
}

static void start_cmd(ahci_dev_t* device)
{
  while (device->port->cmd & 1 << 15);
  device->port->cmd |= 1 << 4;
  device->port->cmd |= 1 << 0;
}

static void send_cmd(ahci_dev_t* device, uint8_t command)
{
  stop_cmd(device);
  device->port->is       = 0;
  device->port->ie       = UINT32_MAX;
  ahci_cmd_hdr_t* header = device->command_header;

  int slot = find_cmdslot(device->port);
  klog("free slot: %d\n", slot);
  if (slot == -1) { abort(); }

  header       += slot;
  header->cfl   = sizeof(ahci_fis_h2d_t) / sizeof(uint32_t);
  header->w     = 0;
  header->prdtl = 0;
  klog("header is set\n");
  ahci_cmd_tbl_t* table = get_cmd_table(header);
  memset(table, 0, sizeof(ahci_cmd_tbl_t));
  ahci_fis_h2d_t* fis = (ahci_fis_h2d_t*)(&table->cfis);
  fis->fis_type       = FIS_TYPE_REG_H2D;
  fis->c              = 1;
  fis->cmd            = 0xEC;
  fis->dev            = device->port_number;
  klog("fis is set\n");
  klog("device is configured\n");
  int spin = 0;
  klog("waiting on device to finish working\n");
  while ((device->port->tfd & 0x88) && spin < 1000000) { spin++; }
  if (spin == 1000000) { abort(); }
  klog("issuing command %x\n", device->port->ie);
  device->port->ci = 1 << slot;
  for (;;)
    {
      klog("Status: 0b%b\n", device->port->is);
      if ((device->port->ci & 1 << slot) == 0) { break; }
      if (device->port->is & 1 << 0)
        {
          klog("received FIS\n");
          halt();
        }
    }
}

static void init_sata_drive(ahci_dev_t* device)
{
  klog("SATA device on port: %d\n", device->port_number);
  send_cmd(device, 0);
  // send IDENTIFY command
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
  device_list            = device;
  switch (device->type)
    {
      case SATA_SIG_ATA:
        {
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

isr_stack_t* ahci_irq(isr_stack_t* stack)
{
  if (!abar) { panic("ABAR uninitialized"); }
  uint32_t is = abar->ghc.is;
  for (int i = 0; i < 32; i++)
    {
      int port_sts = is & (1 << i);
      if (!port_sts) { continue; }
      klog("port %d int\n", i);
      ahci_port_t* port    = &abar->ports[i];
      uint32_t     port_is = port->is;
      if (!port_is) { continue; }
      klog("port %d has result %b\n", i, port_is);
      if (port_is & (0x1F << 26)) { panic("error flag set"); }
      for (int j = 0; j < 32; j++)
        {
          if (port->ci & 1 << j) { klog("Command %d issued\n", j); }
        }
    }
  return stack;
}

static void test_ahci(ahci_port_t* port)
{
  port->is = (uint32_t)-1;
  port->ie = 1;
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
