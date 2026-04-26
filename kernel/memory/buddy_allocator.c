#include "log.h"
#include "stdio.h"
#include "stdlib.h"
#include "types.h"
#include <stdalign.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "buddy_allocator.h"
#define MIN_ALLOC_SIZE 256
#define abort_if_null(x) \
  if (x == NULL)         \
  {                      \
    debug();             \
    abort();             \
  }
struct AllocatorBlock
{
  size_t size;
  bool free;
};

struct BuddyAllocator
{
  struct AllocatorBlock* head;
  size_t size;
  struct AllocatorBlock* tail;
};

static struct BuddyAllocator allocator = {0};

static struct AllocatorBlock* next_block(struct AllocatorBlock* this)
{
  if (this == NULL || this < allocator.head || this >= allocator.tail)
  {
    abort_msg("block is out of bounds");
  }
  if (this->size > allocator.size || this->size < MIN_ALLOC_SIZE)
  {
    abort_msg("corrupted block %x", this);
  }
  return (struct AllocatorBlock*)((char*)this + this->size);
}

static void debug()
{
  struct AllocatorBlock* block = allocator.head;
  while (block < allocator.tail && block->size > 0)
  {
    kernel_log_debug(
      "%x %d %s", block, block->size, block->free ? "free" : "used");
    block = next_block(block);
  }
}

static struct AllocatorBlock* split_block(
  struct AllocatorBlock* block, size_t size)
{
  if (block == NULL || size == 0)
  {
    kernel_log_error("Invalid block or size: %x %d", block, size);
    return NULL;
  }
  while (size <= (block->size >> 1))
  {
    size_t half_size = block->size >> 1;
    block->size      = half_size;
    block            = next_block(block);
    block->size      = half_size;
    block->free      = true;
  }
  // debug();
  if (size <= block->size)
  {
    return block;
  }
  kernel_log_error("Block will not fit into %d bytes", size);
  return NULL;
}

static bool can_merge(
  struct AllocatorBlock* block, struct AllocatorBlock* buddy)
{
  return (block->free && buddy->free && block->size == buddy->size) != 0;
}

static struct AllocatorBlock* find_best(size_t size)
{
  struct AllocatorBlock* block = allocator.head;
  struct AllocatorBlock* next  = next_block(block);
  struct AllocatorBlock* best  = NULL;
  if (next == allocator.tail && block->free)
  {
    return split_block(block, size);
  }

  while (block < allocator.tail && next < allocator.tail)
  {
    if (can_merge(block, next))
    {
      block->size <<= 1;
      if (size <= block->size && (best == NULL || best->size > block->size))
      {
        best = block;
      }
      block = next_block(next);
      if (block < allocator.tail)
      {
        next = next_block(block);
      }
      continue;
    }
    if (block->free && size <= block->size &&
      (best == NULL || best->size > block->size))
    {
      best = block;
    }
    if (next->size > block->size)
    {
      block = next_block(next);
      if (block < allocator.tail)
      {
        next = next_block(block);
      }
      continue;
    }
    block = next;
    next  = next_block(block);
    abort_if_null(next);
  }
  return split_block(best, size);
}

static void merge_blocks()
{
  struct AllocatorBlock* block = allocator.head;
  struct AllocatorBlock* next  = next_block(block);
  while (true)
  {
    bool can_break = true;
    while (block < allocator.tail && next < allocator.tail)
    {
      if (can_merge(block, next))
      {
        block->size <<= 1;
        block         = next_block(block);
        if (block < allocator.tail)
        {
          next      = next_block(block);
          can_break = false;
        }
        continue;
      }
      if (block->size < next->size)
      {
        block = next;
        next  = next_block(block);
        continue;
      }
      block = next_block(next);
      if (block < allocator.tail)
      {
        next = next_block(block);
      }
    }
    if (can_break)
    {
      break;
    }
  }
}

void* buddy_realloc(void* ptr, size_t size)
{
  if (ptr == NULL)
  {
    return buddy_alloc(size);
  }
  struct AllocatorBlock* block =
    (struct AllocatorBlock*)((char*)ptr - sizeof(struct AllocatorBlock));
  struct AllocatorBlock* new_block = find_best(size);
  if (new_block == NULL)
  {
    kernel_log_error("Could not find new block, merging");
    merge_blocks();
    new_block = find_best(size);
    // debug();
  }
  if (new_block == NULL)
  {
    kernel_log_error("Failed to find any valid blocks");
    return NULL;
  }
  memmove(new_block, block, block->size);
  size_t block_size = block->size;
  block->size       = block_size;
  block->free       = true;
  merge_blocks();
  // debug();
  return (void*)((char*)new_block + sizeof(struct AllocatorBlock));
}

void* buddy_alloc(size_t size)
{
  if (size == 0)
  {
    return NULL;
  }
  if (size > MIN_ALLOC_SIZE)
  {
    size = ALIGN_UP((size + sizeof(struct AllocatorBlock)), MIN_ALLOC_SIZE);
  }
  else
  {
    size = MIN_ALLOC_SIZE;
  }
  struct AllocatorBlock* valid_block = find_best(size);
  if (valid_block == NULL)
  {
    kernel_log_error("Could not find a valid block, trying again");
    merge_blocks();
    valid_block = find_best(size);
  }
  if (valid_block == NULL)
  {
    kernel_log_error("Still could not find a valid block, aborting");
    return NULL;
  }
  valid_block->free = false;
  return (void*)((char*)valid_block + sizeof(struct AllocatorBlock));
}

void buddy_free(void* ptr)
{
  if (ptr == NULL)
  {
    kernel_log_error("Cannot free NULL");
    return;
  }
  if ((void*)allocator.head >= ptr || (void*)allocator.tail < ptr)
  {
    kernel_log_error("Memory was not allocated by this allocator");
    return;
  }
  struct AllocatorBlock* block =
    (struct AllocatorBlock*)((char*)ptr - sizeof(struct AllocatorBlock));
  block->free = true;
}

void init_buddy_allocator(uint64_t heap_start, uint64_t heap_size)
{
  struct AllocatorBlock* head = (void*)heap_start;
  head->size                  = heap_size;
  head->free                  = true;
  allocator.head              = head;
  allocator.tail              = (void*)(heap_start + heap_size);
  allocator.size              = heap_size;
  kernel_log_debug(
    "Initialized Buddy Allocator\n\tHead: %x\n\tTail: %x\n\tTag size: %d",
    allocator.head, allocator.tail, sizeof(struct AllocatorBlock));
}
