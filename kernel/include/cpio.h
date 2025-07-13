#ifndef __SOTERIA_KERNEL_CPIO_H__
#define __SOTERIA_KERNEL_CPIO_H__

#include <stdbool.h>
#include <stdint.h>
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

#define CPIO_HEADER_SIZE 26
void init_cpio(uint8_t*);
uint8_t* get_file();
bool next_header();
#endif
