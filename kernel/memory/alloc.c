// NOLINTBEGIN(misc-use-internal-linkage)
#include "memory/alloc.h"
#include "log.h"
#include "paging_internal.h"
#include "stdio.h"
#include <stdalign.h>
#include <stdint.h>
#include "initial_frame_allocator.h"
#include "list_allocator.h"
#ifdef KERNEL_USE_LIMINE

void setup_allocator(struct limine_memmap_response* memory_map)
{
  init_frame_allocator(memory_map);
  init_page_table();
  kernel_log_debug("Mapping pages");
  for (uint64_t page = HEAP_START; page <= HEAP_END; page += PAGE_SIZE)
  {
    uint64_t frame = allocate_frame();
    if (frame == 0)
    {
      abort_msg("Failed to allocate frame");
    }
    __map_page(page, frame, 0b11);
  }
  kernel_log_debug("Finished mapping pages");
  add_region(HEAP_START, HEAP_SIZE);
}
#endif
// NOLINTEND(misc-use-internal-linkage)
