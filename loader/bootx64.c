#include "uefi/uefi.h"
#include "../kernel/include/types.h"
#include "loader.h"

int main()
{
  setup_page_table();
  __attribute__((sysv_abi)) int (*ptr)(kernel_bootinfo_t*, void*);

  ptr = load_kernel();
  if (ptr == NULL)
  {
    printf("Got Null Entry Point\n");
    for (;;);
  }
  kernel_bootinfo_t* bootinfo = get_bootinfo();

  efi_status_t status = BS->ExitBootServices(IM, bootinfo->mmap->key);

  if (status != EFI_SUCCESS)
  {
    for (;;);
  }

  load_page_table();
  ptr(bootinfo, base_address);
}
