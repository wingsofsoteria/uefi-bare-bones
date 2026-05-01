#pragma once
#include <stdint.h>
#ifndef __KERNEL_LOADERS_LOADER_H__
  #error "Do not use lethe_loader.h directly, use loader.h instead"
#endif

enum {
  EfiReservedMemoryType  = 0,  // unusable
  EfiLoaderCode          = 1,  // usable
  EfiLoaderData          = 2,  // usable after kernel init
  EfiBootServicesCode    = 3,  // usable
  EfiBootServicesData    = 4,  // usable
  EfiRuntimeServicesCode = 5,  // maybe usable (runtime services aren't needed)
  EfiRuntimeServicesData = 6,  // maybe usable (runtime services aren't needed)
  EfiConventionalMemory  = 7,  // usable
  EfiUnusableMemory      = 8,  // unusable
  EfiACPIReclaimMemory   = 9,  // usable after init
  EfiACPIMemoryNVS       = 10, // unusable
  EfiMemoryMappedIO      = 11, // unusable
  EfiMemoryMappedIOPortSpace = 12, // unusable
  EfiPalCode                 = 13, // unusable
  EfiPersistentMemory        = 14, // usable
  EfiUnacceptedMemoryType    = 15, // unusable
  EfiMaxMemoryType           = 16, // undefined
};

typedef struct {
  uint32_t type;
  uint32_t __pad;
  uint64_t p_addr;
  uint64_t v_addr;
  uint64_t page_count;
  uint64_t __attrib;
} loader_memory_descriptor_t;

typedef struct {
  loader_memory_descriptor_t* addr;
  uint64_t                    size;
  uint64_t                    desc_size;
  uint32_t                    version;
  uint64_t                    key;
} mmap_t;

typedef struct {
  char     magic[5];
  uint64_t base;
  uint64_t size;
  uint32_t pitch;
  uint32_t horizontal_resolution;
  uint32_t vertical_resolution;
  void*    initfs;
  int      initfs_size;
  mmap_t*  mmap;
  void*    rsdp_address;
  uint64_t stack_top;
  uint64_t stack_bottom;
} kernel_bootinfo_t;
