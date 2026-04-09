// NOLINTBEGIN(misc-use-internal-linkage)
#include "memory/alloc.h"
#include "stdio.h"
#include "types.h"
#include <stdalign.h>
#include <stdint.h>
#include <string.h>

static kernel_page_table_t* page_table = NULL;
static int page_table_len              = 0;
/*
int liballoc_lock()
{
  return 0;
}

int liballoc_unlock()
{
  return 0;
}

void* liballoc_alloc(int pages)
{
  for (int i = 0; i < page_table_len; i++)
  {
    if (!page_table[i].free)
    {
      continue;
    }

    bool consecutive_free = true;

    for (int j = 1; j < pages; j++)
    {
      if (!page_table[i + j].free)
      {
        consecutive_free = false;
      }
    }

    if (consecutive_free)
    {
      for (int j = 0; j < pages; j++)
      {
        page_table[i + j].free = false;
      }

      return (void*)page_table[i].start;
    }
  }

  return NULL;
}

int liballoc_free(void* addr, int pages)
{
  for (int i = 0; i < page_table_len; i++)
  {
    if (page_table[i].start != (uint64_t)addr)
    {
      continue;
    }

    for (int j = 0; j < pages; j++)
    {
      page_table[i + j].free = true;
    }
  }
  return 0;
}
*/
struct ListNode
{
  uint64_t size;
  struct ListNode* next;
};

struct LinkedListAllocator
{
  struct ListNode head;
};

void add_region(struct LinkedListAllocator* allocator, uint64_t start,
  uint64_t size, uint64_t alignment)
{
  if (size < sizeof(struct ListNode))
  {
    abort_msg("Region is too small to mark\n");
  }
  start                 = (start + (alignment - 1)) & ~alignment;
  struct ListNode* node = (void*)start;
  node->next            = allocator->head.next;
  node->size            = size;
  allocator->head.next  = node;
  LOG_DEBUG("Appending Region: %x - %x\n", start, start + size);
}

void* try_alloc(struct ListNode* region, uint64_t size, uint64_t alignment)
{
  uint64_t start      = (uint64_t)region;
  uint64_t region_end = start + region->size;
  start               = (start + (alignment - 1)) & ~alignment;
  uint64_t end        = start + size;
  if (end > region_end)
  {
    LOG_DEBUG("Region is too small for allocation of size %d\n", size);
    return NULL;
  }
  uint64_t excess = end - region_end;
  if (excess > 0 && excess < sizeof(struct ListNode))
  {
    LOG_DEBUG("Region does not have enough excess space for a ListNode\n");
    return NULL;
  }
  LOG_DEBUG("Alloc: addr:%x size:%d excess:%d\n", start, size, excess);
  return (void*)start;
}

struct ListNode* get_valid_region(
  struct LinkedListAllocator* allocator, uint64_t size, uint64_t alignment)
{
  struct ListNode* current = &allocator->head;
  while (current->next != NULL)
  {
    void* alloc_result = try_alloc(current->next, size, alignment);
    if (alloc_result == NULL)
    {
      current = current->next;
      continue;
    }
    struct ListNode* valid_region = current->next;

    current->next = valid_region->next;
    return valid_region;
  }

  return NULL;
}

void* alloc(struct LinkedListAllocator* allocator, uint64_t size)
{
  uint64_t required_size  = size + sizeof(struct ListNode);
  uint64_t alignment      = alignof(struct ListNode);
  struct ListNode* region = get_valid_region(allocator, size, alignment);
  if (region == NULL)
  {
    return NULL;
  }
  uint64_t alloc_end  = (uint64_t)region + required_size;
  uint64_t region_end = (uint64_t)region + region->size;
  uint64_t excess     = region_end - alloc_end;
  if (excess > 0)
  {
    add_region(allocator, alloc_end, excess, alignment);
  }
  uint64_t alloc_start = (((uint64_t)region) + (alignment - 1)) & ~alignment;
  return (void*)alloc_start;
}

struct InitialFrameAllocator
{
  void* memory_map;
  next_usable_ptr next_usable;
  struct Frame next;
};

uint64_t allocate_frame(struct InitialFrameAllocator* allocator)
{
  if (allocator->next.start)
  {
    LOG_DEBUG("Frame was uninitialized");
    return 0;
  }
  if (allocator->next.base == 0)
  {
    allocator->next.base   += 4096;
    allocator->next.length -= 4096;
    LOG_DEBUG("Frame base was 0, adjusting to %l", allocator->next.base);
  }
  if (allocator->next.length < 4096)
  {
    LOG_DEBUG("Frame is full, moving to next frame");
    allocator->next = allocator->next_usable(allocator->memory_map);
  }
  uint64_t addr = allocator->next.base;
  LOG_DEBUG("Allocating frame:\n\taddr: %x\n\tlength: %l", allocator->next.base,
    allocator->next.length);
  allocator->next.base   += 4096;
  allocator->next.length -= 4096;
  return addr;
}

uint64_t canonical_address(uint64_t physical_address)
{
  int64_t signed_addr        = (int64_t)(physical_address << 16);
  uint64_t canonical_address = signed_addr >> 16;
  LOG_DEBUG("Canonical Address: %x", canonical_address);
  return canonical_address;
}

kernel_page_table_t* early_new_page_table(
  struct InitialFrameAllocator* allocator, kernel_page_table_t* table,
  uint16_t index, uint16_t flags)
{
  uint64_t entry = table->pages[index];
  bool created   = false;
  LOG_DEBUG("Entry value: %b", entry);
  if (entry == 0)
  {
    uint64_t frame = allocate_frame(allocator);
    if (frame == 0)
    {
      abort_msg("Failed to allocate page table");
    }
    entry   = frame | flags;
    created = true;
  }
  else
  {
    entry |= flags;
  }
  LOG_DEBUG("New entry value: %b", entry);
  table->pages[index]                 = entry;
  void* entry_ptr                     = (void*)(entry + hhdm_mapping);
  kernel_page_table_t* new_page_table = (kernel_page_table_t*)entry_ptr;

  if (created)
  {
    LOG_DEBUG("Zeroing out page table");
    memset(new_page_table, 0, sizeof(kernel_page_table_t));
  }

  return new_page_table;
}

void early_map_page(struct InitialFrameAllocator* allocator, uint64_t page,
  uint64_t frame, uint16_t flags)
{
  LOG_DEBUG("Attempting to map frame %x to page %x", frame, page);
  uint64_t default_flags = 0b111;
  uint16_t p4_index      = page >> 39;
  uint16_t p3_index      = page >> 30;
  uint16_t p2_index      = page >> 21;
  uint64_t p1_index      = page >> 12;
  kernel_page_table_t* p3 =
    early_new_page_table(allocator, page_table, p4_index, default_flags);
  kernel_page_table_t* p2 =
    early_new_page_table(allocator, p3, p3_index, default_flags);
  kernel_page_table_t* p1 =
    early_new_page_table(allocator, p2, p2_index, default_flags);
  if (p1->pages[p1_index] != 0)
  {
    abort_msg("Page is already mapped");
  }
  LOG_DEBUG("Setting p1 entry %b to %b", p1->pages[p1_index], frame | flags);
  p1->pages[p1_index] = frame | flags;
}

void setup_allocator(
  next_usable_ptr loader_specific_memory_function, void* memory_map)
{
  struct InitialFrameAllocator allocator = (struct InitialFrameAllocator){
    memory_map, loader_specific_memory_function, (struct Frame){true, 0, 0}};
  allocator.next     = allocator.next_usable(allocator.memory_map);
  uint64_t test_heap = 0x444444440000;
  test_heap          = PAGE_ALIGN(test_heap);
  uint64_t test_size = 100 * 1024;
  uint64_t heap_end  = test_heap + test_size - 1;
  heap_end           = PAGE_ALIGN(heap_end);
  page_table         = (void*)(read_cr3() + hhdm_mapping);

  LOG_DEBUG("Mapping pages");
  for (uint64_t page = test_heap; page <= heap_end; page += 4096)
  {
    uint64_t frame = allocate_frame(&allocator);
    if (frame == 0)
    {
      abort_msg("Failed to allocate frame");
    }
    early_map_page(&allocator, page, frame, 0b111);
  }
  LOG_DEBUG("Finished mapping pages");
  struct LinkedListAllocator late_allocator = {
    .head = (struct ListNode){.size = 0, .next = NULL}};

  add_region(&late_allocator, test_heap, test_size, alignof(struct ListNode));
  /*dummy_alloc(mmap);

  if (page_table == (void*)-1)
  {
    printf("failed to allocate page table\n");
  }

  int current_page = 0;

  for (int i = 0; i < (mmap->size / mmap->desc_size); i++)
  {
    loader_memory_descriptor_t* desc =
      (loader_memory_descriptor_t*)((uint8_t*)mmap->addr +
        (i * mmap->desc_size));

    if (!is_usable(desc->type))
    {
      continue;
    }

    for (int j = 0; j < desc->page_count; j++, current_page++)
    {
      page_table[current_page].start = desc->v_addr + (4096 * j);

      page_table->free = is_available(desc->type);
    }
  }*/
}
// NOLINTEND(misc-use-internal-linkage)
