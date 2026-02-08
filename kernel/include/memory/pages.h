#ifndef __KERNEL_MEMORY_PAGES_H__
#define __KERNEL_MEMORY_PAGES_H__

#include <stdint.h>

void setup_page_table();
void map_pages(void* v_addr, void* p_addr, uint64_t pages, uint16_t flags);
#define ADDR_TO_ENTRY(ADDR, FLAG)                               \
  (((uint64_t)ADDR >> 12) & 0xFFFFFFFFFF) << 12 | (0b1 | FLAG);

#endif
