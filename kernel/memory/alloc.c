// NOLINTBEGIN(misc-use-internal-linkage)
#include "memory/alloc.h"
#include "stdio.h"
#include <stdalign.h>
#include <stdint.h>

static kernel_page_table_t* page_table = (void*)-1;

static int page_table_len = 0;

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
    return 0;
  }
  if (allocator->next.base == 0)
  {
    allocator->next.base   += 4096;
    allocator->next.length -= 4096;
  }
  if (allocator->next.length < 4096)
  {
    allocator->next = allocator->next_usable(allocator->memory_map);
  }
  uint64_t addr           = allocator->next.base;
  allocator->next.base   += 4096;
  allocator->next.length -= 4096;
  return addr;
}

uint64_t canonical_address(uint64_t physical_address)
{
  int64_t signed_addr        = (int64_t)(physical_address << 16);
  uint64_t canonical_address = signed_addr >> 16;
  LOG_DEBUG("Canonical Address: %x\n", canonical_address);
  return canonical_address;
}

//! I'm choosing to keep track of everything but the EfiReserved memory type
//! because it contains so many pages that we'd run out of memory trying to
//! track them all (and we can't even use them)
void setup_allocator(
  next_usable_ptr loader_specific_memory_function, void* memory_map)
{
  struct InitialFrameAllocator allocator = (struct InitialFrameAllocator){
    memory_map, loader_specific_memory_function, (struct Frame){true, 0, 0}};
  allocator.next     = allocator.next_usable(allocator.memory_map);
  uint64_t test_heap = 0x444444440000;
  uint64_t test_size = 100 * 1024;
  canonical_address(test_heap);
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
