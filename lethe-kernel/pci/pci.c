#include "pci/pci.h"

#include "config.h"
#include "log.h"
#include "memory/alloc.h"
#include "memory/paging.h"
#include "pci_internal.h"
#include "stdbool.h"
#include "stddef.h"
#include "types.h"
#include "utils.h"

#include <stdint.h>

/*
 * Steps:
 * if MCFG table is present; break
 * else if UEFI PCI bus support protocol is present; goto mechanism #1
 * else abort
 * */

struct pci_bus
{
  struct pci_device* parent;
  struct pci_device* dev_list;
  struct pci_bus*    next;
  uint8_t            index;
};

struct pci_device
{
  struct pci_device* parent_device;
  struct pci_bus*    parent_bus;
  struct pci_device* next;
  struct pci_device* functions;
  uint8_t            index;
};

static struct pci_device* new_device(struct pci_bus* bus, uint8_t index)
{
  struct pci_device* device = kmalloc(sizeof(struct pci_device));
  device->parent_device     = NULL;
  device->parent_bus        = bus;
  device->index             = index;
  device->functions         = NULL;
  device->next              = bus->dev_list;
  bus->dev_list             = device;
  return device;
}

static struct pci_device* new_function(
  struct pci_bus*    bus,
  struct pci_device* device,
  uint8_t            index
)
{
  struct pci_device* function = kmalloc(sizeof(struct pci_device));
  function->functions         = NULL;
  function->parent_device     = device;
  function->parent_bus        = bus;
  function->index             = index;
  function->next              = device->functions;
  device->functions           = function;
  return function;
}

static struct pci_bus* new_bus(struct pci_device* device, uint8_t index)
{
  struct pci_bus* bus = kmalloc(sizeof(struct pci_bus));
  bus->index          = index;
  bus->dev_list       = NULL;
  bus->parent         = device;
  bus->next           = NULL;
  return bus;
}

static struct pci_bus* root_bus()
{
  static struct pci_bus* root = NULL;
  if (root == NULL) { root = new_bus(NULL, 0); }
  return root;
}

static struct pci_device* root_device()
{
  static struct pci_device* root = NULL;
  if (root == NULL) { root = new_device(root_bus(), 0); }
  return root;
}

static uint32_t get_pci_device_register(
  uint64_t bus,
  uint8_t  device,
  uint8_t  function,
  uint8_t  reg
)
{
  uint32_t value;
  if (kernel_config.has_mcfg) // I only really need mmio accesses for segment
                              // group > 1 (eg bus > 255)
    {
      uint64_t ecam_base_addr = get_mmio_ecam_addr(bus);
      uint64_t physical_address =
        ecam_base_addr + (bus << 20 | device << 15 | function << 12);
      void* configuration_space = (void*)(physical_address + hhdm_mapping);
      value = *(volatile uint32_t*)(configuration_space + (reg * 4));
    }
  else
    {
      uint32_t lbus      = (uint32_t)bus;
      uint32_t ldevice   = (uint32_t)device;
      uint32_t lfunction = (uint32_t)function;
      uint8_t  offset    = reg * 4;
      uint32_t address = (uint32_t)(lbus << 16 | ldevice << 11 |
                                    lfunction << 8 | (offset & 0xFC) | 1 << 31);
      outl(CONFIG_ADDRESS, address);
      value = inl(CONFIG_DATA);
    }
  return value;
}

static struct pci_device_descriptor get_device_descriptor(
  uint8_t bus,
  uint8_t device,
  uint8_t function
)
{
  struct pci_device_descriptor pci_device;
  for (int i = 0; i < 0x10; i++)
    {
      pci_device.registers[i].bits_32 =
        get_pci_device_register(bus, device, function, i);
    }
  if ((pci_device.registers[3].bits_8.upper_mid & 0x7F) == 0x2)
    {
      pci_device.registers[0x10].bits_32 =
        get_pci_device_register(bus, device, function, 0x10);
      pci_device.registers[0x11].bits_32 =
        get_pci_device_register(bus, device, function, 0x11);
    }

  return pci_device;
}

static void check_function(struct pci_device*, struct pci_device_descriptor);

static void check_device(struct pci_bus* bus, uint8_t device)
{
  uint32_t register_0 = get_pci_device_register(bus->index, device, 0, 0);
  uint16_t vendor_id  = register_0 & 0xFFFF;
  if (vendor_id == 0xFFFF) return;
  klog(
    "got device at bus %d device %d function 0: %0.8x\n",
    bus->index,
    device,
    register_0
  );
  struct pci_device*           self = new_device(bus, device);
  struct pci_device_descriptor desc =
    get_device_descriptor(bus->index, device, 0);
  check_function(self, desc);
  uint8_t header_type = desc.registers[3].bits_8.upper_mid;
  if (header_type & 0x80)
    {
      klog("device is multi-function\n");
      for (int function = 1; function < 8; function++)
        {
          desc      = get_device_descriptor(bus->index, device, function);
          vendor_id = desc.registers[0].bits_16.lower;
          if (vendor_id == 0xFFFF) continue;
          struct pci_device* func = new_function(bus, self, function);
          klog(
            "got device at bus %d device %d function %d: %0.8x\n",
            bus->index,
            device,
            function,
            desc.registers[0].bits_32
          );
          check_function(func, desc);
        }
    }
}

static void check_bus(struct pci_device* parent, uint8_t bus)
{
  struct pci_bus* bus_structure = new_bus(parent, bus);
  for (int device = 0; device < 32; device++)
    {
      check_device(bus_structure, device);
    }
}

static void check_function(
  struct pci_device*           self,
  struct pci_device_descriptor desc
)
{
  uint8_t base_class = desc.registers[2].bits_8.upper;
  uint8_t sub_class  = desc.registers[2].bits_8.upper_mid;

  if ((base_class == 0x6) && (sub_class == 0x4))
    {
      uint8_t secondary_bus = desc.registers[6].bits_8.lower_mid;
      klog("got secondary bus %d\n", secondary_bus);
      check_bus(self, secondary_bus);
    }
}

static void scan_pci_space()
{
  struct pci_device_descriptor desc        = get_device_descriptor(0, 0, 0);
  uint8_t                      header_type = desc.registers[3].bits_8.upper_mid;
  if (header_type & 0x80)
    {
      for (int i = 0; i < 8; i++)
        {
          desc               = get_device_descriptor(0, 0, i);
          uint16_t vendor_id = desc.registers[0].bits_16.lower;
          if (vendor_id == 0xFFFF) break;
          struct pci_device* function =
            new_function(root_bus(), root_device(), i);
          klog(
            "got device at bus 0 device 0 function %x: %0.8x\n",
            i,
            desc.registers[0].bits_32
          );
          check_bus(function, i);
        }
    }
  else
    {
      check_bus(root_device(), 0);
    }
}

bool init_pci()
{
  scan_pci_space();
  return true;
}
