#pragma once
#include "loaders/loader.h"

#include <stdint.h>
#include <types.h>

void* kmalloc(size_t);
void  kfree(void*);
void* krealloc(void*, size_t);
#ifdef KERNEL_USE_LIMINE
void setup_allocator(struct limine_memmap_response*);
#else
void setup_allocator(mmap_t*);
#endif
