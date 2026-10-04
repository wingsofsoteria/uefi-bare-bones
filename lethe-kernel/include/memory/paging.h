#pragma once
#include <stdint.h>
#define PAGE_SIZE          4096
#define PAGE_COUNT         512
#define PAGE_PRESENT       (1 << 0)
#define PAGE_WRITABLE      (1 << 1)
#define PAGE_USER          (1 << 2)
#define PAGE_WRITE_THROUGH (1 << 3)
#define PAGE_CACHE_DISABLE (1 << 4)
void     unmap_page(uint64_t page);
uint64_t virtual_to_physical(uint64_t virtual);
void     map_page(uint64_t virtual, uint64_t physical, uint16_t flags);
uint64_t map_contiguous_pages(
  uint64_t virtual,
  uint64_t physical,
  uint64_t num_pages,
  uint16_t flags
);
uint64_t map_page_nearest(uint64_t virtual, uint64_t physical, uint16_t flags);
