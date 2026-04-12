#pragma once
#include <stdint.h>
#define PAGE_SIZE  4096
#define PAGE_COUNT 512
void unmap_page(uint64_t virt);
void map_page(uint64_t virt, uint64_t phys, uint16_t flags);
