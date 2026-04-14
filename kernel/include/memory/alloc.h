#pragma once
#include "loaders/loader.h"
#include <stdint.h>
#include <types.h>

#define HEAP_START 0xffffffff81000000
#define HEAP_SIZE  0x20000
#define HEAP_END   0xffffffff81020000

void* kmalloc(size_t);
void kfree(void*);
void* krealloc(void*, size_t);
#ifdef KERNEL_USE_LIMINE
void setup_allocator(struct limine_memmap_response*);
#else
void setup_allocator(mmap_t*);
#endif
