#include "initial_frame_allocator.h"

#include "log.h"
#include "memory/paging.h"
#include "stdio.h"
#include "types.h"
#include "utils.h"

#include <string.h>
static frame_allocator_t allocator = { 0 };
#ifdef KERNEL_USE_LIMINE
static void next_usable()
{
  struct limine_memmap_response* mmap    = allocator.memory_map;
  struct limine_memmap_entry**   entries = mmap->entries;
  for (int i = 0; i < mmap->entry_count; i++)
    {
      struct limine_memmap_entry* entry = entries[i];
      if (entry->type == LIMINE_MEMMAP_USABLE)
        {
          allocator.next = (frame_t){ .start = false,
            .base                            = entry->base,
            .length                          = entry->length };
          return;
        }
    }
  abort_msg("Could not find a free entry");
}

uint64_t allocate_frame()
{
  if (allocator.next.start)
    {
      kernel_log_error("Frame was uninitialized");
      return 0;
    }
  if (allocator.next.base == 0)
    {
      allocator.next.base   += 4096;
      allocator.next.length -= 4096;
      kernel_log_error(
        "Frame base was 0, adjusting to %#lx",
        allocator.next.base
      );
    }
  if (allocator.next.length < 4096)
    {
      kernel_log_error("Frame is full, moving to next frame");
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
