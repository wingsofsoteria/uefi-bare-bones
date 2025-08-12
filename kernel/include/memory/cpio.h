#ifndef __KERNEL_MEMORY_CPIO_H__
#define __KERNEL_MEMORY_CPIO_H__

#include <stdbool.h>
#include <stdint.h>

#define CPIO_HEADER_SIZE 26
void init_cpio(uint8_t*);
uint8_t* get_file();
bool next_header();
#endif
