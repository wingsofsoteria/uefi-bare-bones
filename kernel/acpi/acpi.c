// SSDT and DSDT require an AML parser which I am putting off as long as
// possible BGRT is purely for cosmetic reasons

#include "acpi.h"
#include "acpispec/tables.h"
#include "cpu/sleep.h"
#include "lai/core.h"
#include "lai/helpers/pm.h"
#include "lai/helpers/sci.h"
#include "log.h"
#include "memory/alloc.h"
#include "stdio.h"
#include "stdlib.h"
#include "types.h"
#include <string.h>
static acpi_xsdt_t* XSDT = NULL;
// NOLINTNEXTLINE
void laihost_log(int level, const char* msg)
{
  puts(msg);
  putchar('\n');
}

__attribute__((noreturn)) void laihost_panic(const char* msg)
{
  abort_msg("PANIC: %s\n", msg);
}

void* laihost_malloc(size_t size)
{
  void* addr = kmalloc(size);
  return addr;
}
// NOLINTNEXTLINE
void* laihost_realloc(void* oldptr, size_t newsize, size_t oldsize)
{
  return krealloc(oldptr, newsize);
}
// NOLINTNEXTLINE
void laihost_free(void* ptr, size_t size)
{
  kfree(ptr);
}
/*void* laihost_map(size_t address, size_t count)
{
  abort();
  return (void*)address;
}
void laihost_unmap(void* ptr, size_t count)
{
  abort();
}*/

static bool sdt_checksum(acpi_header_t* sdt)
{
  uint8_t* bytes = (uint8_t*)sdt;
  uint8_t sum    = 0;
  for (int i = 0; i < sdt->length; i++)
  {
    sum += bytes[i];
  }
  return sum == 0;
}

static void acpi_dump_tables()
{
  int entries = (XSDT->header.length - sizeof(acpi_header_t)) / 8;
  for (int i = 0; i < entries; i++)
  {
    acpi_header_t* sdt = (void*)(XSDT->tables[i] + hhdm_mapping);
    kernel_log_debug("ACPI Table %.4s", sdt->signature);
  }
}

void* laihost_scan(const char* sig, size_t index)
{
  int current_index = 0;
  if (strncmp(sig, "DSDT", 4) == 0)
  {
    acpi_fadt_t* fadt = laihost_scan("FACP", 0);
    return (void*)(fadt->x_dsdt + hhdm_mapping);
  }
  int entries = (XSDT->header.length - sizeof(acpi_header_t)) / 8;
  for (int i = 0; i < entries; i++)
  {
    acpi_header_t* sdt = (void*)(XSDT->tables[i] + hhdm_mapping);
    kernel_log_debug("ACPI Table %.4s", sdt->signature);
    if (strncmp(sig, sdt->signature, 4) == 0)
    {
      if (current_index != index)
      {
        current_index++;
        continue;
      }
      return sdt;
    }
  }
  kernel_log_error("Could not find table with signature %s", sig);
  return NULL;
}
/*
void laihost_outb(uint16_t port, uint8_t val)
{
  asm volatile("outb %b0, %1"
    :
    : "a"(val), "Nd"(port)
    : "memory");
}

void laihost_outw(uint16_t port, uint16_t val)
{
  asm volatile("outw %w0, %1"
    :
    : "a"(val), "Nd"(port)
    : "memory");
}

void laihost_outd(uint16_t port, uint32_t val)
{
  asm volatile("out %0,%1"
    :
    : "a"(val), "Nd"(port)
    : "memory");
}

uint8_t laihost_inb(uint16_t port)
{
  uint8_t val;
  asm volatile("inb %1, %b0"
    : "=a"(val)
    : "Nd"(port)
    : "memory");
  return val;
}

uint16_t laihost_inw(uint16_t port)
{
  uint16_t val;
  asm volatile("inw %1, %w0"
    : "=a"(val)
    : "Nd"(port)
    : "memory");
  return val;
}

uint32_t laihost_ind(uint16_t port)
{
  uint32_t val;
  asm volatile("in %1, %0"
    : "=a"(val)
    : "Nd"(port)
    : "memory");
  return val;
}
*/
/* TODO PCI host functions */

void laihost_sleep(uint64_t ms)
{
  ksleep((kernel_duration_t){.milliseconds = ms});
}

/* TODO laihost_timer, laihost_handle_global_notify, laihost_handle_amldebug */

int acpi_early_init(void* rsdp_pointer)
{
  acpi_xsdp_t* rsdp = rsdp_pointer;

  XSDT = (void*)(rsdp->xsdt + hhdm_mapping);
  if (!sdt_checksum(&XSDT->header))
  {
    kernel_log_error("Failed to parse ACPI tables\n");
    abort();
  }
  lai_enable_tracing(7);
  lai_set_acpi_revision(rsdp->revision);
  lai_create_namespace();
  madt_init();
  lapic_init();
  //  ignore scis
  // lai_set_sci_event(0);
  return 0;
}

int acpi_late_init()
{
  lai_enable_acpi(1);
  kernel_log_debug("ACPI initialization finished\n");
  return 0;
}

void shutdown()
{
  lai_enter_sleep(5);
}
