// clang-format Language: C
#ifndef __KERNEL_TYPES_H__
#define __KERNEL_TYPES_H__
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#define KERNEL_START 0xFFFF800000000000
extern int kernel_initialization;

#define VIRTUAL(phys) (phys + KERNEL_START)
typedef struct
{
  uint16_t c_magic;
  uint16_t c_dev;
  uint16_t c_ino;
  uint16_t c_mode;
  uint16_t c_uid;
  uint16_t c_gid;
  uint16_t c_nlink;
  uint16_t c_rdev;
  uint16_t c_mtime[2];
  uint16_t c_namesize;
  uint16_t c_filesize[2];
} cpio_header_t;

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
  uint32_t rsdt_address;
  uint64_t xsdt_address;
  uint8_t* bitmap;
} kernel_bootinfo_t;

#endif
