#include "uefi/uefi.h"
#include <stdint.h>
#include <stdbool.h>
#include "loader.h"
#define ADDR_TO_ENTRY(ADDR, FLAG)                               \
  (((uint64_t)ADDR >> 12) & 0xFFFFFFFFFF) << 12 | (0b1 | FLAG);
typedef struct
{
  uint64_t pages[512];
} page_table_t;

static page_table_t* p4_table;

void* quick_allocate_page()
{
  uint64_t addr;
  BS->AllocatePages(AllocateAnyPages, EfiLoaderData, 1, &addr);
  return (void*)addr;
}

page_table_t* new_page_table()
{
  page_table_t* page = quick_allocate_page();
  memset(page, 0, EFI_PAGE_SIZE);
  if (page == NULL)
  {
    printf("FAILED TO ALLOCATE PAGE TABLE\n");
    return NULL;
  }
  return page;
}

void map_pages(void* v_addr, void* p_addr, uint64_t pages, uint16_t flags)
{
  for (int j = 0; j < pages; j++)
  {
    if ((uint64_t)v_addr % EFI_PAGE_SIZE != 0)
    {
      return;
    }
    else if ((uint64_t)p_addr % EFI_PAGE_SIZE != 0)
    {
      return;
    }
    uint64_t page_v_addr = (uint64_t)v_addr + (j * EFI_PAGE_SIZE);
    uint64_t page_p_addr = (uint64_t)p_addr + (j * EFI_PAGE_SIZE);
    uint64_t index       = page_v_addr >> 12;
    uint64_t p1_index    = index & 0b111111111;
    uint64_t p2_index    = (index >> 9) & 0b111111111;
    uint64_t p3_index    = (index >> 18) & 0b111111111;
    uint64_t p4_index    = (index >> 27) & 0b111111111;
    uint64_t entry       = p4_table->pages[p4_index];
    page_table_t* p3_table;
    if ((entry & 0b1) == 0)
    {
      p3_table                  = new_page_table();
      entry                     = ADDR_TO_ENTRY(p3_table, flags);
      p4_table->pages[p4_index] = entry;
    }
    else
    {
      p3_table = (page_table_t*)(entry & 0x000ffffffffff000);
    }
    entry = p3_table->pages[p3_index];
    page_table_t* p2_table;
    if ((entry & 0b1) == 0)
    {
      p2_table                  = new_page_table();
      entry                     = ADDR_TO_ENTRY(p2_table, flags);
      p3_table->pages[p3_index] = entry;
    }
    else
    {
      p2_table = (page_table_t*)(entry & 0x000ffffffffff000);
    }
    entry = p2_table->pages[p2_index];
    page_table_t* p1_table;
    if ((entry & 0b1) == 0)
    {
      p1_table                  = new_page_table();
      entry                     = ADDR_TO_ENTRY(p1_table, flags);
      p2_table->pages[p2_index] = entry;
    }
    else
    {
      p1_table = (page_table_t*)(entry & 0x000ffffffffff000);
    }
    entry                     = ADDR_TO_ENTRY(page_p_addr, flags);
    p1_table->pages[p1_index] = entry;
  }
}

void copy_existing_pages()
{
  p4_table = new_page_table();
  page_table_t* cr3;
  asm volatile("mov %%cr3, %0"
    : "=r"(cr3));
  for (uint64_t i = 0; i < 512; i++)
  {
    p4_table->pages[i] = cr3->pages[i];
  }
}

mmap_t quick_memory_map()
{
  uint64_t mmap_size = 0, desc_size = 0, map_key = 0;
  uint32_t desc_version;
  efi_status_t status;
  efi_memory_descriptor_t* memory_map = NULL;
  while (1)
  {
    status = BS->GetMemoryMap(
      &mmap_size, memory_map, &map_key, &desc_size, &desc_version);
    if (status == EFI_SUCCESS)
      return (mmap_t){(loader_memory_descriptor_t*)memory_map, mmap_size,
        desc_size, desc_version, map_key};
    status = BS->AllocatePool(EfiLoaderData, mmap_size, (void**)&memory_map);
  }
}

void load_page_table()
{
  asm volatile("mov %0, %%cr3"
    :
    : "r"((uint64_t)p4_table));
}

void setup_page_table()
{
  copy_existing_pages();
  mmap_t mmap = quick_memory_map();
  for (int i = 0; i < (mmap.size / mmap.desc_size); i++)
  {
    efi_memory_descriptor_t* desc =
      (efi_memory_descriptor_t*)((uint64_t)mmap.addr + (i * mmap.desc_size));
    uint64_t v_addr = KERNEL_START + desc->PhysicalStart;
    map_pages(
      (void*)v_addr, (void*)desc->PhysicalStart, desc->NumberOfPages, 0b10);
  }

  BS->FreePool(mmap.addr);
  load_page_table();
}
