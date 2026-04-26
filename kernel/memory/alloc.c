#include "memory/alloc.h"
#include "buddy_allocator.h"
#include "log.h"
#include "memory/paging.h"
#include "paging_internal.h"
#include "stdio.h"
#include <stdalign.h>
#include <stdint.h>
#include "initial_frame_allocator.h"
#ifdef KERNEL_USE_LIMINE
extern int _kernel_end_addr;
static uint64_t heap_start = 0x444444440000;
static uint64_t heap_size  = 0x20000;
void* kmalloc(size_t size)
{
  return buddy_alloc(size);
}

void kfree(void* ptr)
{
  return buddy_free(ptr);
}

void* krealloc(void* ptr, size_t size)
{
  return buddy_realloc(ptr, size);
}

void setup_allocator(struct limine_memmap_response* memory_map)
{
  // heap_start = (uint64_t)&_kernel_end_addr + 0x10000;
  heap_start = ALIGN_UP(heap_start, PAGE_SIZE);
  init_frame_allocator(memory_map);
  init_page_table();
  kernel_log_debug("Mapping pages");
  for (uint64_t page  = heap_start; page <= (heap_start + heap_size);
    page             += PAGE_SIZE)
  {
    uint64_t frame = allocate_frame();
    if (frame == 0)
    {
      abort_msg("Failed to allocate frame");
    }
    map_page(page, frame, 0b11);
  }
  kernel_log_debug("Finished mapping pages");
  init_buddy_allocator(heap_start, heap_size);
}
#endif
