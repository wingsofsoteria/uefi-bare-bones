#include "log.h"
#include "memory/alloc.h"
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
struct AllocatorBlock
{
  size_t size;
  bool free;
};

struct BuddyAllocator
{
  struct AllocatorBlock* head;
  struct AllocatorBlock* tail;
};

static struct BuddyAllocator allocator = {0};

static struct AllocatorBlock* next_block(struct AllocatorBlock* this)
{
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
    kernel_log_debug("Invalid block or size: %x %d", block, size);
    return NULL;
  }
  while (size <= (block->size >> 1))
  {
    size_t half_size = block->size >> 1;
    kernel_log_debug("Split block %x into %d", block, half_size);
    block->size = half_size;
    block       = next_block(block);
    block->size = half_size;
    block->free = true;
  }
  kernel_log_debug("New block %x %d", block, block->size);
  debug();
  if (size <= block->size)
  {
    return block;
  }
  kernel_log_debug("Block will not fit into %d bytes", size);
  return NULL;
}

static bool can_merge(
  struct AllocatorBlock* block, struct AllocatorBlock* buddy)
{
  return block->free && buddy->free && block->size == buddy->size;
}

static struct AllocatorBlock* find_best(size_t size)
{
  kernel_log_debug("Locating block with at least size %d", size);
  struct AllocatorBlock* block = allocator.head;
  struct AllocatorBlock* next  = next_block(block);
  struct AllocatorBlock* best  = NULL;
  kernel_log_debug("Block: %x - Next: %x", block, next);
  if (next == allocator.tail && block->free)
  {
    kernel_log_debug("Splitting block into %d halves", size);
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
  kernel_log_debug("Reallocating block %x", ptr);
  struct AllocatorBlock* block =
    (struct AllocatorBlock*)((char*)ptr - sizeof(struct AllocatorBlock));
  kernel_log_debug(
    "Found block %x %d %s", block, block->size, block->free ? "free" : "used");
  struct AllocatorBlock* new_block = find_best(size);
  if (new_block == NULL)
  {
    kernel_log_debug("Could not find new block, merging");
    merge_blocks();
    new_block = find_best(size);
    debug();
  }
  if (new_block == NULL)
  {
    kernel_log_error("Failed to find any valid blocks");
    return NULL;
  }
  kernel_log_debug("Found replacement block %x %d %s", new_block,
    new_block->size, new_block->free ? "free" : "used");

  memmove(new_block, block, block->size);
  size_t block_size = block->size;
  block->size       = block_size;
  block->free       = true;
  kernel_log_debug("Merging blocks");
  merge_blocks();
  debug();
  return new_block;
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
    kernel_log_debug("Could not find a valid block, trying again");
    merge_blocks();
    valid_block = find_best(size);
  }
  if (valid_block == NULL)
  {
    kernel_log_debug("Still could not find a valid block, aborting");
    return NULL;
  }
  kernel_log_debug("Found valid block %x, offsetting by %d for tag",
    valid_block, sizeof(struct AllocatorBlock));
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
  if ((void*)allocator.head > ptr || (void*)allocator.tail < ptr)
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
  allocator.tail              = (struct AllocatorBlock*)(HEAP_END);
  kernel_log_debug(
    "Initialized Buddy Allocator\n\tHead: %x\n\tTail: %x\n\tTag size: %d",
    allocator.head, allocator.tail, sizeof(struct AllocatorBlock));
}
