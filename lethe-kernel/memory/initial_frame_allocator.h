#pragma once

#include "loaders/loader.h"

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
  bool     start;
  uint64_t base;
  uint64_t length;
} frame_t;

typedef struct
{
  void*   memory_map;
  frame_t next;
} frame_allocator_t;

uint64_t allocate_frame();
#ifdef KERNEL_USE_LIMINE
void init_frame_allocator(struct limine_memmap_response* memory_map);
#endif
