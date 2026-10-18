// message signaled interrupts scratchpad
//
//

#include "acpi/pic.h"
#include "cpu/idt.h"
#include "cpu/isr.h"
#include "log.h"
#include "pci/pci.h"

#include <stdint.h>
#include <stdio.h>

#define MSI_DATA_REG(x) (x + 0x3) // TODO confirm MSI data register offset
#define MSI_ADDR_REG(x) (x + 0x1)
#define MSI_CTRL_REG(x) (x) // LITERALLY ONLY FOR READABILITY

void     lapic_write(uint16_t offset, uint32_t value);
uint32_t lapic_read(uint16_t offset);

int enable_msi(void* device_handle, int msi_cap_offset, interrupt handler)
{
  uint8_t  msi_cap_reg = msi_cap_offset / 4;
  uint32_t register_0  = get_register_value(device_handle, msi_cap_reg);
  if ((register_0 & 0xFF) == 0x11)
    {
      return 0; // TODO enable msi-x
    }
  klog("S/C: %x\n", get_register_value(device_handle, 0x1));
  klog("Int: %16b\n", get_register_value(device_handle, 0xF));
  // TODO actually make sure that we have free interrupts to use
  // I am selecting to ignore everything except the vector field until it
  // becomes absolutely necessary
  lapic_write(0x310, 0);
  set_register_value(device_handle, MSI_DATA_REG(msi_cap_reg), 0x79);
  // Not quite sure if I need to send the lapic addr or the offset to the
  // command register
  set_register_value(
    device_handle,
    MSI_ADDR_REG(msi_cap_reg),
    lapic_addr_phys() + 0x300
  );
  // only setting the enable bit, offset by 16 to account for cap_id and ptr
  // set_register_value(device_handle, MSI_CTRL_REG(msi_cap_reg), 1 << 16);
  set_register_value(
    device_handle,
    MSI_CTRL_REG(msi_cap_reg),
    register_0 & 0xFFFF
  );
  register_handler(0x79, handler);
  return 1;
}

int check_msi_capability(void* device_handle)
{
  uint32_t status_reg = get_register_value(device_handle, 0x1);
  if (!(status_reg & (1 << 20))) return 0;
  uint32_t cap_reg = get_register_value(device_handle, 0xD);
  uint8_t  cap_ptr = (cap_reg & 0xFC);
  uint32_t msi_cap = 0;
  for (int i = 0; cap_ptr != 0; i++)
    {
      klog("capabilities offset: %x\n", cap_ptr / 4);
      uint32_t pci_cap = get_register_value(device_handle, cap_ptr / 4);
      klog("%x\n", pci_cap);
      if ((pci_cap & 0xFF) == 0x5 || (pci_cap & 0xFF) == 0x11)
        {
          msi_cap = cap_ptr;
          break;
        }
      cap_ptr = ((pci_cap & 0xFF00) >> 8);
    }

  return msi_cap;
}

