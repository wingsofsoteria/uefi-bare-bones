#include "log.h"
#include "memory/paging.h"
#include "pci/pci.h"
#include "types.h"

void init_ahci()
{
  void* controller_handle = locate_pci_device(0x1, 0x6);
  if (!pci_compare_value(controller_handle, 3, 0, 0x007F0000))
    {
      klog(
        "got header type: 0x%x instead of 0x0\n",
        get_register_value(controller_handle, 3)
      );
      return;
    }
  uint32_t abar         = get_register_value(controller_handle, 0x9);
  uint64_t virtual_abar = map_page_nearest(
    abar + hhdm_mapping,
    abar,
    PAGE_PRESENT | PAGE_WRITABLE | PAGE_CACHE_DISABLE
  );
  klog("AHCI Base Address Register: %x\n", abar);
}
