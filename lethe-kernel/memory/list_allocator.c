#include "list_allocator.h"
#include "initial_frame_allocator.h"
#include "log.h"
#include <stddef.h>
#include "memory/paging.h"
#include "stdio.h"
#include "utils.h"
#include <stdalign.h>
#include <stdint.h>
static list_allocator_t allocator = {0};
static uint64_t         heap_end  = 0;
static int              debug     = 0;
// NOLINTNEXTLINE
void set_heap_end(uint64_t val) { heap_end = val; }

static void increase_heap_size(int pages)
{
  uint64_t heap_start   = heap_end + PAGE_SIZE;
  uint64_t new_heap_end = heap_start + (PAGE_SIZE * pages);
  if (debug) {
    kernel_log_debug("%#lx -> %#lx", heap_end, new_heap_end);
  }
  for (uint64_t i = heap_start; i <= new_heap_end; i += PAGE_SIZE) {
    uint64_t frame = allocate_frame();
    if (frame == 0) {
      abort_msg("Out of Memory");
    }
    map_page(i, frame, 0b11);
  }
  heap_end = new_heap_end;
  add_region(heap_start, pages);
}

void add_region(uint64_t start, int pages)
{
  if (debug) {
    kernel_log_debug("Adding region %#lx - %#lx", start,
                     start + (PAGE_SIZE * pages));
  }
  list_node_t node      = {0};
  node.next             = allocator.head.next;
  node.pages            = pages;
  list_node_t* node_ptr = (void*)start;
  *node_ptr             = node;
  allocator.head.next   = node_ptr;
}

static void* try_alloc(list_node_t* region, int pages)
{
  if (pages > region->pages) {
    kernel_log_error("Region is too small for allocation of size %d\n",
                     pages * PAGE_SIZE);
    return NULL;
  }
  uint64_t excess = region->pages - pages;
  return (void*)region;
}

static list_node_t* get_valid_region(int pages)
{
  list_node_t* current = &allocator.head;
  while (current->next != NULL) {
    void* alloc_result = try_alloc(current->next, pages);
    if (alloc_result == NULL) {
      current = current->next;
      continue;
    }
    list_node_t* valid_region = current->next;

    current->next = valid_region->next;
    return valid_region;
  }

  return NULL;
}

void* list_alloc(int pages)
{
  list_node_t* region = get_valid_region(pages);
  if (region == NULL) {
    increase_heap_size(pages);
    region = get_valid_region(pages);
    if (region == NULL) {
      abort();
    }
  }
  int      excess    = region->pages - pages;
  uint64_t alloc_end = (uint64_t)region + (excess * PAGE_SIZE);
  if (excess > 0) {
    add_region(alloc_end, excess);
  }
  if (debug) {
    kernel_log_debug("Alloc: %d pages\nRegion: %d Excess: %d", pages,
                     region->pages, excess);
  }
  return (void*)(region);
}
