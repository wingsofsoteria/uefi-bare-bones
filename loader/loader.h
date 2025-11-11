#ifndef __LOADER_LOADER_H__
#define __LOADER_LOADER_H__
#include "../kernel/include/types.h"
#include <stdint.h>

extern void* base_address;
mmap_t quick_memory_map();
kernel_bootinfo_t* get_bootinfo();
void* load_kernel();
void map_pages(void* v_addr, void* p_addr, uint64_t pages, uint16_t flags);
void setup_page_table();
void load_page_table();
#endif
