#include "types.h"
#include <stdlib.h>
#include <uacpi/kernel_api.h>
#include <stdio.h>
static void* kernel_rsdp = NULL;
// NOLINTNEXTLINE
void kernel_rsdp_from_bootinfo(void* address)
{
  kernel_rsdp = address;
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

uacpi_status uacpi_kernel_io_map(
  uacpi_io_addr base, uacpi_size len, uacpi_handle* out)
{
  abort();
}

void uacpi_kernel_io_unmap(uacpi_handle handle)
{
  abort();
}

void uacpi_kernel_free(void* mem)
{
  free(mem);
}

void* uacpi_kernel_alloc(uacpi_size size)
{
  return malloc(size);
}

void uacpi_kernel_pci_device_close(uacpi_handle handle)
{
  abort();
}

uacpi_u64 uacpi_kernel_get_nanoseconds_since_boot()
{
  abort();
  return 0;
}

uacpi_status uacpi_kernel_schedule_work(
  uacpi_work_type work, uacpi_work_handler handler, uacpi_handle ctx)
{
  abort();
}

uacpi_handle uacpi_kernel_create_spinlock()
{
  abort();
}

void uacpi_kernel_free_spinlock(uacpi_handle handle)
{
  abort();
}

void uacpi_kernel_stall(uacpi_u8 usec)
{
  abort();
}

uacpi_status uacpi_kernel_io_read8(
  uacpi_handle handle, uacpi_size offset, uacpi_u8* value)
{
  abort();
}

uacpi_status uacpi_kernel_io_read16(
  uacpi_handle handle, uacpi_size offset, uacpi_u16* value)
{
  abort();
}

uacpi_status uacpi_kernel_io_read32(
  uacpi_handle handle, uacpi_size offset, uacpi_u32* value)
{
  abort();
}

uacpi_status uacpi_kernel_io_write8(
  uacpi_handle handle, uacpi_size offset, uacpi_u8 value)
{
  abort();
}

uacpi_status uacpi_kernel_io_write16(
  uacpi_handle handle, uacpi_size offset, uacpi_u16 value)
{
  abort();
}
uacpi_status uacpi_kernel_io_write32(
  uacpi_handle handle, uacpi_size offset, uacpi_u32 value)
{
  abort();
}

uacpi_thread_id uacpi_kernel_get_thread_id()
{
  abort();
}

uacpi_cpu_flags uacpi_kernel_lock_spinlock(uacpi_handle handle)
{
  abort();
}

void uacpi_kernel_unlock_spinlock(uacpi_handle handle, uacpi_cpu_flags flags)
{
  abort();
}

uacpi_status uacpi_kernel_wait_for_work_completion()
{
  abort();
}

uacpi_handle uacpi_kernel_create_mutex()
{
  abort();
}

void uacpi_kernel_free_mutex(uacpi_handle handle)
{
  abort();
}

void uacpi_kernel_free_event(uacpi_handle handle)
{
  abort();
}

void uacpi_kernel_sleep(uacpi_u64 msec)
{
  abort();
}

uacpi_bool uacpi_kernel_wait_for_event(uacpi_handle handle, uacpi_u16 value)
{
  abort();
}

void uacpi_kernel_reset_event(uacpi_handle handle)
{
  abort();
}

uacpi_status uacpi_kernel_acquire_mutex(uacpi_handle handle, uacpi_u16 value)
{
  abort();
}

void uacpi_kernel_release_mutex(uacpi_handle handle)
{
  abort();
}

uacpi_status uacpi_kernel_install_interrupt_handler(uacpi_u32 irq,
  uacpi_interrupt_handler handler, uacpi_handle ctx,
  uacpi_handle* out_irq_handle)
{
  abort();
}

uacpi_status uacpi_kernel_uninstall_interrupt_handler(
  uacpi_interrupt_handler handler, uacpi_handle irq_handle)
{
  abort();
}

uacpi_status uacpi_kernel_pci_device_open(
  uacpi_pci_address address, uacpi_handle* out_handle)
{
  abort();
}

uacpi_status uacpi_kernel_pci_read32(
  uacpi_handle device, uacpi_size offset, uacpi_u32* out)
{
  abort();
}

uacpi_status uacpi_kernel_pci_read16(
  uacpi_handle device, uacpi_size offset, uacpi_u16* out)
{
  abort();
}

uacpi_status uacpi_kernel_pci_read8(
  uacpi_handle device, uacpi_size offset, uacpi_u8* out)
{
  abort();
}

uacpi_status uacpi_kernel_pci_write32(
  uacpi_handle device, uacpi_size offset, uacpi_u32 value)
{
  abort();
}

uacpi_status uacpi_kernel_pci_write16(
  uacpi_handle device, uacpi_size offset, uacpi_u16 value)
{
  abort();
}

uacpi_status uacpi_kernel_pci_write8(
  uacpi_handle device, uacpi_size offset, uacpi_u8 value)
{
  abort();
}

uacpi_handle uacpi_kernel_create_event()
{
  abort();
}

void uacpi_kernel_signal_event(uacpi_handle handle)
{
  abort();
}
#ifdef UACPI_FORMATTED_LOGGING
void uacpi_kernel_log(uacpi_log_level level, const uacpi_char* format, ...)
{
  va_list args;
  va_start(args, format);
  vprintf(format, args);
  va_end(args);
}

void uacpi_kernel_vlog(
  uacpi_log_level level, const uacpi_char* format, uacpi_va_list args)
{
  vprintf(format, args);
}
#else
void uacpi_kernel_log(uacpi_log_level level, const uacpi_char* format)
{
  printf(format);
}
#endif

uacpi_status uacpi_kernel_handle_firmware_request(
  uacpi_firmware_request* request)
{
  abort();
}
