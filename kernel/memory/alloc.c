#include "memory/alloc.h"
#include "list_allocator.h"
#include "log.h"
#include "memory/paging.h"
#include "paging_internal.h"
#include "stdio.h"
#include <stdalign.h>
#include <stdint.h>
#include "initial_frame_allocator.h"
#include "liballoc.h"
#include "types.h"
#ifdef KERNEL_USE_LIMINE
extern int _kernel_end_addr;
static uint64_t heap_start = 0x444444440000;
static uint64_t heap_size  = 0x20000;
void* kmalloc(size_t size)
{
  return malloc(size);
}

void kfree(void* ptr)
{
  return free(ptr);
}

void* krealloc(void* ptr, size_t size)
{
  return realloc(ptr, size);
}

int liballoc_lock()
{
  return 0;
}

int liballoc_unlock()
{
  return 0;
}

int liballoc_free(void* ptr, int pages)
{
  add_region((uint64_t)ptr, pages);
  return 0;
}

void* liballoc_alloc(int pages)
{
  kernel_log_debug("allocating %d pages", pages);
  return list_alloc(pages);
}

void setup_allocator(struct limine_memmap_response* memory_map)
{
  heap_start = (uint64_t)&_kernel_end_addr + 0x10000;
  heap_start = ALIGN_UP(heap_start, PAGE_SIZE);
  init_frame_allocator(memory_map);
  init_page_table();
  add_region(heap_start, heap_size / PAGE_SIZE);
}
#endif
