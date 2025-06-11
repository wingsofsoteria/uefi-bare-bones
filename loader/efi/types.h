#ifndef __EFI_TYPES_H__
#define __EFI_TYPES_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef void *efi_handle_t;
typedef uint64_t efi_status_t;
typedef uint64_t efi_uint_t;

static const uint64_t MAX_BIT = 0x8000000000000000ULL;

#define ERROR_CODE(status) (MAX_BIT | (status))

static const efi_status_t EFI_SUCCESS = 0;
static const efi_status_t EFI_LOAD_ERROR = ERROR_CODE(1);
static const efi_status_t EFI_INVALID_PARAMETER = ERROR_CODE(2);
static const efi_status_t EFI_UNSUPPORTED = ERROR_CODE(3);
static const efi_status_t EFI_BUFFER_TOO_SMALL = ERROR_CODE(5);

struct efi_time {
  uint16_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
  uint8_t pad1;
  uint32_t nanosecond;
  int16_t time_zone;
  uint8_t daylight;
  uint8_t pad2;
};

struct efi_guid {
  uint32_t data1;
  uint16_t data2;
  uint16_t data3;
  uint8_t data4[8];
};

struct efi_table_header {
  uint64_t signature;
  uint32_t revision;
  uint32_t header_size;
  uint32_t crc32;
  uint32_t reserved;
};

struct efi_memory_descriptor {
  uint32_t type;
  uint64_t physical_start;
  uint64_t virtual_start;
  uint64_t pages;
  uint64_t attributes;
};

enum efi_allocate_type {
  EFI_ALLOCATE_ANY_PAGES,
  EFI_ALLOCATE_MAX_ADDRESS,
  EFI_ALLOCATE_ADDRESS,
  EFI_MAX_ALLOCATE_TYPE,
};

enum efi_memory_type {
  EFI_RESERVED_MEMORY_TYPE,
  EFI_LOADER_CODE,
  EFI_LOADER_DATA,
  EFI_BOOT_SERVICES_CODE,
  EFI_BOOT_SERVICES_DATA,
  EFI_RUNTIME_SERVICES_CODE,
  EFI_RUNTIME_SERVICES_DATA,
  EFI_CONVENTIAL_MEMORY,
  EFI_UNUSABLE_MEMORY,
  EFI_ACPI_RECLAIM_MEMORY,
  EFI_ACPI_MEMORY_NVS,
  EFI_MEMORY_MAPPED_IO,
  EFI_MEMORY_MAPPED_IO_PORT_SPACE,
  EFI_PAL_CODE,
  EFI_PERSISTENT_MEMORY,
  EFI_MAX_MEMORY_TYPE,
};

#endif  // __EFI_TYPES_H__
