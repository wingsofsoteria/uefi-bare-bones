#ifndef __KERNEL_MEMORY_PAGES_H__
#define __KERNEL_MEMORY_PAGES_H__
#include <stdint.h>
void setup_page_table();
void map_pages(void* v_addr, void* p_addr, uint64_t pages, uint16_t flags);


#endif
