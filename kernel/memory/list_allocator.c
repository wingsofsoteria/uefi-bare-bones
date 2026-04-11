#include "list_allocator.h"
#include "log.h"
#include "stdio.h"
#include "types.h"
#include <stdalign.h>
static list_allocator_t allocator = {0};
void add_region(uint64_t start, uint64_t size)
{
  if (size < sizeof(list_node_t))
  {
    abort_msg("Region is too small to mark\n");
  }
  if (start != ALIGN_UP(start, alignof(list_node_t)))
  {
    abort_msg("Address %x is not aligned", start);
  }
  kernel_log_debug("Adding region %x to %x", start, start + size);
  list_node_t node      = {0};
  node.next             = allocator.head.next;
  node.size             = size;
  list_node_t* node_ptr = (void*)start;
  *node_ptr             = node;
  allocator.head.next   = node_ptr;
}

static void* try_alloc(list_node_t* region, uint64_t size, uint64_t alignment)
{
  uint64_t start      = (uint64_t)region;
  uint64_t region_end = start + region->size;
  start               = ALIGN_UP(start, alignment);
  uint64_t end        = start + size;
  if (end > region_end)
  {
    kernel_log_debug("Region is too small for allocation of size %d\n", size);
    return NULL;
  }
  uint64_t excess = end - region_end;
  if (excess > 0 && excess < sizeof(list_node_t))
  {
    kernel_log_error(
      "Region does not have enough excess space for a ListNode\n");
    return NULL;
  }
  kernel_log_debug("Alloc: addr:%x size:%d excess:%d\n", start, size, excess);
  return (void*)start;
}

static list_node_t* get_valid_region(uint64_t size, uint64_t alignment)
{
  list_node_t* current = &allocator.head;
  while (current->next != NULL)
  {
    void* alloc_result = try_alloc(current->next, size, alignment);
    if (alloc_result == NULL)
    {
      current = current->next;
      continue;
    }
    list_node_t* valid_region = current->next;

    current->next = valid_region->next;
    return valid_region;
  }

  return NULL;
}

void* list_alloc(uint64_t size)
{
  uint64_t required_size = size + sizeof(list_node_t);
  uint64_t alignment     = alignof(list_node_t);
  list_node_t* region    = get_valid_region(size, alignment);
  if (region == NULL)
  {
    return NULL;
  }
  uint64_t alloc_end  = (uint64_t)region + required_size;
  uint64_t region_end = (uint64_t)region + region->size;
  uint64_t excess     = region_end - alloc_end;
  if (excess > 0)
  {
    add_region(alloc_end, excess);
  }
  uint64_t alloc_start = ALIGN_UP((uint64_t)region, alignment);
  return (void*)alloc_start;
}
