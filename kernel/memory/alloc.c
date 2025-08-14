#include "memory/alloc.h"
#include <stdio.h>

static kernel_page_table_t* page_table = (void*)-1;

static int page_table_len = 0;

bool never_usable(int type)
{
  return type == EfiReservedMemoryType || type == EfiUnusableMemory || type == EfiACPIMemoryNVS || type == EfiUnacceptedMemoryType || type == EfiPalCode || type == EfiMemoryMappedIO || type == EfiMemoryMappedIOPortSpace;
}
bool is_usable(int type)
{
  bool usable           = !never_usable(type);
  bool initially_usable = usable && type != EfiLoaderData && type != EfiACPIReclaimMemory;

  return kernel_initialization == -255 ? initially_usable : usable;
}
void dummy_alloc(mmap_t* mmap)
{
  uint64_t total_pages = 0;
  for (int i = 0; i < (mmap->size / mmap->desc_size); i++)
  {
    loader_memory_descriptor_t* desc = (loader_memory_descriptor_t*)((uint8_t*)mmap->addr + (i * mmap->desc_size));

    if (!never_usable(desc->type))
    {
      total_pages += desc->page_count;
    }
    if (desc->type == EfiLoaderData && is_usable(desc->type))
    {
      printf("Failed sanity check, init flag set to: %d", kernel_initialization);
      for (;;);
    }
  }
  printf("Max memory: %x\n", total_pages * 4096);
  int size_in_pages = (total_pages * sizeof(kernel_page_table_t)) / 4096;

  for (int i = 0; i < (mmap->size / mmap->desc_size); i++)
  {
    loader_memory_descriptor_t* desc = (loader_memory_descriptor_t*)((uint8_t*)mmap->addr + (i * mmap->desc_size));

    if (!is_usable(desc->type))
    {
      continue;
    }

    if (desc->page_count >= size_in_pages)
    {
      desc->type     = EfiLoaderData;
      page_table     = (void*)desc->v_addr;
      page_table_len = total_pages;
      return;
    }
  }
}

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

//! I'm choosing to keep track of everything but the EfiReserved memory type because it contains so many pages that we'd run out of memory trying to track them all (and we can't even use them)

void setup_allocator(mmap_t* mmap)
{
  dummy_alloc(mmap);

  if (page_table == (void*)-1)
  {
    printf("failed to allocate page table\n");
  }

  int current_page = 0;

  for (int i = 0; i < (mmap->size / mmap->desc_size); i++)
  {
    loader_memory_descriptor_t* desc = (loader_memory_descriptor_t*)((uint8_t*)mmap->addr + (i * mmap->desc_size));

    if (never_usable(desc->type))
    {
      continue;
    }

    for (int j = 0; j < desc->page_count; j++, current_page++)
    {
      page_table[current_page].start = desc->v_addr + (4096 * j);

      page_table->free = is_usable(desc->type);
    }
  }
  printf("Finished Loading Memory\n");
}
