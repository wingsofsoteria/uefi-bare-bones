#ifndef __KERNEL_MEMORY_H__
#define __KERNEL_MEMORY_H__
#include "types.h"
void setup_allocator(mmap_t* mmap);
void setup_page_table(mmap_t* mmap);

#endif
