#include "pci/pci.h"

#include "config.h"
#include "log.h"
#include "pci_internal.h"
#include "stdbool.h"
#include "types.h"
#include "utils.h"

#include <stdint.h>

/*
 * Steps:
 * if MCFG table is present; break
 * else if UEFI PCI bus support protocol is present; goto mechanism #1
 * else abort
 * */

static uint32_t get_pci_device_register(
  uint64_t bus,
  uint8_t  device,
  uint8_t  function,
  uint8_t  reg
)
{
  uint32_t value;
  if (bus > 255) // I only really need mmio accesses for segment group > 1 (eg
                 // bus > 255)
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

static void check_function(struct pci_device_descriptor);

static void check_device(uint8_t bus, uint8_t device)
{
  uint32_t register_0 = get_pci_device_register(bus, device, 0, 0);
  uint16_t vendor_id  = register_0 & 0xFFFF;
  if (vendor_id == 0xFFFF) return;
  klog(
    "got device at bus %d device %d function 0: %0.8x\n",
    bus,
    device,
    register_0
  );
  struct pci_device_descriptor desc = get_device_descriptor(bus, device, 0);
  check_function(desc);
  uint8_t header_type = desc.registers[3].bits_8.upper_mid;
  if (header_type & 0x80)
    {
      klog("device is multi-function\n");
      for (int function = 1; function < 8; function++)
        {
          desc      = get_device_descriptor(bus, device, function);
          vendor_id = desc.registers[0].bits_16.lower;
          if (vendor_id == 0xFFFF) continue;

          klog(
            "got device at bus %d device %d function %d: %0.8x\n",
            bus,
            device,
            function,
            desc.registers[0].bits_32
          );
          check_function(desc);
        }
    }
}

static void check_bus(uint8_t bus)
{
  for (int device = 0; device < 32; device++) { check_device(bus, device); }
}

static void check_function(struct pci_device_descriptor desc)
{
  uint8_t base_class = desc.registers[2].bits_8.upper;
  uint8_t sub_class  = desc.registers[2].bits_8.upper_mid;

  if ((base_class == 0x6) && (sub_class == 0x4))
    {
      uint8_t secondary_bus = desc.registers[6].bits_8.lower_mid;
      klog("got secondary bus %d\n", secondary_bus);
      check_bus(secondary_bus);
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
          klog(
            "got device at bus 0 device 0 function %x: %0.8x\n",
            i,
            desc.registers[0].bits_32
          );
          check_bus(i);
        }
    }
  else
    {
      check_bus(0);
    }
}

bool init_pci()
{
  scan_pci_space();
  return true;
}
