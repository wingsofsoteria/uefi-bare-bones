#include "initial_frame_allocator.h"

#include "memory/paging.h"
#include "types.h"
#include "utils.h"

#include <string.h>
static frame_allocator_t allocator = { 0 };
#ifdef KERNEL_USE_LIMINE
  #define KERNEL_USED 0xA9

  // #define DEBUG
  #ifdef DEBUG
    #include "log.h"
  #endif
static void next_usable()
{
  struct limine_memmap_response* mmap    = allocator.memory_map;
  struct limine_memmap_entry**   entries = mmap->entries;
  for (int i = 0; i < mmap->entry_count; i++)
    {
      struct limine_memmap_entry* entry = entries[i];
  #ifdef DEBUG
      klog("%llx %llu %llu\n", entry->base, entry->length, entry->type);
  #endif
      if (entry->type == LIMINE_MEMMAP_USABLE)
        {
          entry->type    = KERNEL_USED;
          allocator.next = (frame_t){ .start = false,
            .base                            = entry->base,
            .length                          = entry->length };
          return;
        }
    }
  panic("Could not find a free entry\n");
}

uint64_t allocate_frame()
{
  #ifdef DEBUG
  klog("%llx %llu\n", allocator.next.base, allocator.next.length);
  #endif
  if (allocator.next.start)
    {
  #ifdef DEBUG
      klog("Frame was uninitialized\n");
  #endif
      return 0;
    }
  if (allocator.next.base == 0)
    {
      allocator.next.base   += 4096;
      allocator.next.length -= 4096;
  #ifdef DEBUG
      klog("Frame base was 0, adjusting to %#llx\n", allocator.next.base);
  #endif
    }
  if (allocator.next.length < 4096)
    {
  #ifdef DEBUG
      klog("Frame is full, moving to next frame\n");
  #endif
      next_usable();
      return allocate_frame();
    }
  uint64_t addr          = allocator.next.base;
  allocator.next.base   += 4096;
  allocator.next.length -= 4096;

  char* ptr = (void*)(addr + hhdm_mapping);
  memset(ptr, 0, PAGE_SIZE);
  return addr;
}

void init_frame_allocator(struct limine_memmap_response* memory_map)
{
  allocator.memory_map = memory_map;
  allocator.next       = (frame_t){ .start = true, .base = 0, .length = 0 };
  next_usable();
}
#endif
