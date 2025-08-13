// clang-format Language: C
#ifndef __KERNEL_MEMORY_ALLOC_H__
#define __KERNEL_MEMORY_ALLOC_H__

#include "types.h"
void setup_allocator(mmap_t* mmap);

enum
{
  EfiReservedMemoryType      = 0,  // unusable
  EfiLoaderCode              = 1,  // usable
  EfiLoaderData              = 2,  // usable after kernel init
  EfiBootServicesCode        = 3,  // usable
  EfiBootServicesData        = 4,  // usable
  EfiRuntimeServicesCode     = 5,  // maybe usable (runtime services aren't needed)
  EfiRuntimeServicesData     = 6,  // maybe usable (runtime services aren't needed)
  EfiConventionalMemory      = 7,  // usable
  EfiUnusableMemory          = 8,  // unusable
  EfiACPIReclaimMemory       = 9,  // usable after init
  EfiACPIMemoryNVS           = 10, // unusable
  EfiMemoryMappedIO          = 11, // unusable
  EfiMemoryMappedIOPortSpace = 12, // unusable
  EfiPalCode                 = 13, // unusable
  EfiPersistentMemory        = 14, // usable
  EfiUnacceptedMemoryType    = 15, // unusable
  EfiMaxMemoryType           = 16, // undefined
};

typedef struct
{
  uint64_t start;

  bool free;

} kernel_page_table_t;

#endif
