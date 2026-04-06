#ifndef __KERNEL_TYPES_H__
#define __KERNEL_TYPES_H__

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#ifdef KERNEL_USE_LIMINE
  #define KERNEL_START 0xffffffff80000000
#else
  #define KERNEL_START 0xFFFF800000000000
#endif
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
#endif
