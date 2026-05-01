#pragma once
#include <stdint.h>
#define PAGE_SIZE  4096
#define PAGE_COUNT 512

#define PAGE_WRITABLE (1 << 1)
#define PAGE_USER     (1 << 2)
void     unmap_page(uint64_t);
uint64_t virtual_to_physical(uint64_t);
void     map_page(uint64_t, uint64_t, uint16_t);
