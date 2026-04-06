#ifndef __KERNEL_LOADERS_LETHE_LOADER_H__
#define __KERNEL_LOADERS_LETHE_LOADER_H__

#include <stdint.h>
#ifndef __KERNEL_LOADERS_LOADER_H__
  #error "Do not use lethe_loader.h directly, use loader.h instead"
#endif

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
  loader_memory_descriptor_t* addr;
  uint64_t size;
  uint64_t desc_size;
  uint32_t version;
  uint64_t key;
} mmap_t;

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
  mmap_t* mmap;
  void* rsdp_address;
  uint64_t stack_top;
  uint64_t stack_bottom;
} kernel_bootinfo_t;

#endif
