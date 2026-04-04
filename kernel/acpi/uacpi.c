#include "types.h"
#include "uacpi/status.h"
#include <stdlib.h>
#include <uacpi/kernel_api.h>
static void* kernel_rsdp = NULL;
// NOLINTNEXTLINE
void kernel_rsdp_from_bootinfo(kernel_bootinfo_t* bootinfo)
{
  kernel_rsdp = bootinfo->rsdp_address;
}

uacpi_status uacpi_kernel_get_rsdp(uacpi_phys_addr* out_rsdp_address)
{
  out_rsdp_address = kernel_rsdp;
  return UACPI_STATUS_OK;
}

void* uacpi_kernel_map(uacpi_phys_addr addr, uacpi_size len)
{
  return (void*)addr;
}

void uacpi_kernel_unmap(void* addr, uacpi_size len)
{
}

void uacpi_kernel_free(void* mem)
{
  free(mem);
}
