#ifndef __COMMON_BOOTINFO_H__
#define __COMMON_BOOTINFO_H__
#include <stdint.h>
#include "kernel/include/cpio.h"
typedef struct
{
  uint32_t type;
  uint32_t __pad;
  uint64_t p_addr;
  uint64_t v_addr;
  uint64_t page_count;
  uint64_t __attrib;
} loader_memory_descriptor_t;
typedef struct
{
  char magic[5];
  uint64_t base;
  uint64_t size;
  uint32_t pitch;
  uint32_t horizontal_resolution;
  uint32_t vertical_resolution;
  void* initfs;
  int initfs_size;
  loader_memory_descriptor_t* mmap;
  uint64_t mmap_size;
  uint64_t desc_size;
  uint32_t rsdt_address;
  uint64_t xsdt_address;
} kernel_bootinfo_t;

typedef struct
{
  char signature[8];
  uint8_t checksum;
  char oemid[6];
  uint8_t revision;
  uint32_t rsdt_address;
  uint32_t length;
  uint64_t xsdt_address;
  uint8_t extended_checksum;
  char reserved[3];
} acpi_rsdp_structure_t;

#endif
