// SSDT and DSDT require an AML parser which I am putting off as long as
// possible BGRT is purely for cosmetic reasons

#include "acpi.h"
#include "acpispec/tables.h"
#include "cpu/sleep.h"
#include "lai/core.h"
#include "lai/helpers/sci.h"
#include "lai/host.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static acpi_xsdt_t* XSDT = NULL;

void laihost_log(int level, const char* msg)
{
  char* level_str = "";
  switch (level)
  {
    case LAI_DEBUG_LOG:
      {
        level_str = "DEBUG: ";
        break;
      }
    case LAI_WARN_LOG:
      {
        level_str = "WARN: ";
        break;
      }
  }
  printf("%s%s\n", level_str, msg);
}

__attribute__((noreturn)) void laihost_panic(const char* msg)
{
  printf("PANIC: %s\n", msg);
  abort();
}

void* laihost_malloc(size_t size)
{
  return malloc(size);
}

void* laihost_realloc(void* oldptr, size_t newsize, size_t oldsize)
{
  return realloc(oldptr, newsize);
}

void laihost_free(void* ptr, size_t size)
{
  return free(ptr);
}

void* laihost_map(size_t address, size_t count)
{
  return (void*)address;
}

void laihost_unmap(void* ptr, size_t count)
{
}

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

void* acpi_get_table(const char id[4])
{
  int entries = (XSDT->header.length - sizeof(acpi_header_t)) / 8;
  for (int i = 0; i < entries; i++)
  {
    acpi_header_t* sdt = (void*)XSDT->tables[i];
    if (id[0] != sdt->signature[0] || id[1] != sdt->signature[1] ||
      id[2] != sdt->signature[2] || id[3] != sdt->signature[3])
    {
      continue;
    }
    return sdt;
  }

  return NULL;
}

void* laihost_scan(const char* sig, size_t index)
{
  if (__builtin_strncmp(sig, "DSDT", 4) == 0)
  {
    acpi_fadt_t* fadt = laihost_scan("FACP", 0);
    return (void*)fadt->x_dsdt;
  }
  int entries = (XSDT->header.length - sizeof(acpi_header_t)) / 8;
  for (int i = 0; i < entries; i++)
  {
    acpi_header_t* sdt = (void*)XSDT->tables[i];
    if (__builtin_strncmp(sig, sdt->signature, 4) == 0)
    {
      return sdt;
    }
  }
  abort();
}

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

/* TODO PCI host functions */

void laihost_sleep(uint64_t ms)
{
  ksleep((kernel_duration_t){.milliseconds = ms});
}

/* TODO laihost_timer, laihost_handle_global_notify, laihost_handle_amldebug */

int acpi_init(void* rsdp_pointer)
{
  acpi_xsdp_t* rsdp = rsdp_pointer;

  XSDT = (void*)VIRTUAL(rsdp->xsdt);
  if (!sdt_checksum(&XSDT->header))
  {
    printf("Failed to parse ACPI tables\n");
    abort();
  }
  lai_set_acpi_revision(rsdp->revision);
  lai_create_namespace();
  //  ignore scis
  // lai_set_sci_event(0);
  // lai_enable_acpi(1);
  printf("ACPI initialization finished\n");
  madt_init();
  lapic_init();
  return 0;
}
