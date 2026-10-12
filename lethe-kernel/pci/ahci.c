#include "log.h"
#include "memory/paging.h"
#include "pci/pci.h"
#include "types.h"

#include <stdint.h>

#define SATA_SIG_ATA   0x00000101 // SATA drive
#define SATA_SIG_ATAPI 0xEB140101 // SATAPI drive
#define SATA_SIG_SEMB  0xC33C0101 // Enclosure management bridge
#define SATA_SIG_PM    0x96690101 // Port multiplier

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

void debug_ahci(ahci_mem_t* abar)
{
  int        i   = 0;
  ahci_cap_t cap = abar->ghc.cap;
  klog(
    "Host capabilities\ns64a %b\nsncq %b\nssntf %b\nsmps %b\nsss %b\nsalp "
    "%b\nsal %b\nsclo %b\niss %b\nsam %b\nspm %b\nfbss %b\npmd %b\nssc %b\npsc "
    "%b\nncs %b\ncccs %b\nems %b\n",
    cap.s64a,
    cap.sncq,
    cap.ssntf,
    cap.smps,
    cap.sss,
    cap.salp,
    cap.sal,
    cap.sclo,
    cap.iss,
    cap.sam,
    cap.spm,
    cap.fbss,
    cap.pmd,
    cap.ssc,
    cap.psc,
    cap.ncs,
    cap.cccs,
    cap.ems
  );
}

static void init_sata_drive(ahci_port_t* port)
{
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
}

static void init_port(ahci_port_t* port)
{
  if ((port->ssts & 0b111) != 0x3) return;
  switch (port->sig)
    {
      case SATA_SIG_ATA:
        {
          init_sata_drive(port);
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
      init_port(&abar->ports[i]);
    }
}

static void enumerate_ahci_controllers(int max)
{
  for (int i = 0; i < max; i++)
    {
      void* controller_handle = locate_pci_device(0x1, 0x6, i);
      if (!controller_handle) break;
      if (!pci_compare_value(controller_handle, 3, 0, 0x007F0000))
        {
          klog(
            "got header type: 0x%x instead of 0x0\n",
            get_register_value(controller_handle, 3)
          );
          break;
        }
      uint32_t    phys_addr = get_register_value(controller_handle, 0x9);
      ahci_mem_t* abar      = (void*)map_page_nearest(
        phys_addr + hhdm_mapping,
        phys_addr,
        PAGE_PRESENT | PAGE_WRITABLE | PAGE_CACHE_DISABLE
      );
      klog("AHCI Base Address Register: %p\n", abar);
      debug_ahci(abar);
      enumerate_ahci_devices(abar);
    }
}

void init_ahci()
{
  enumerate_ahci_controllers(1);
  klog(
    "debug: %zx %zx %zx %zx\n",
    sizeof(ahci_ghc_t),
    sizeof(ahci_cap_t),
    sizeof(ahci_port_t),
    sizeof(ahci_mem_t)
  );
}
