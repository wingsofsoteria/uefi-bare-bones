#ifndef __KERNEL_MEMORY_ALLOC_H__
#define __KERNEL_MEMORY_ALLOC_H__

#include <stdint.h>
#include <types.h>
typedef struct
{
  uint64_t start;

  bool free;

} kernel_page_table_t;

struct Frame
{
  bool start;
  uint64_t base;
  uint64_t length;
};

typedef struct Frame (*next_usable_ptr)(void*);
void setup_allocator(next_usable_ptr, void*);

#endif
