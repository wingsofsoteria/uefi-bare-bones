#include <memory.h>
#include <debug.h>
#include <stdlib.h>
#include <string.h>
#include <types.h>
#include <stdint.h>
#include <stdbool.h>
#define ADDR_TO_ENTRY(ADDR, FLAG) (((uint64_t)ADDR >> 12) & 0xFFFFFFFFFF) << 12 | (0b1 | FLAG);
typedef struct
{
  uint64_t pages[512];
} __attribute__((packed)) page_table_t;

static page_table_t* p4_table;

static inline void load_page_table()
{
  asm volatile("mov %0, %%cr3"
    :
    : "r"((uint64_t)p4_table));
}

page_table_t* new_page_table()
{
  page_table_t* page = malloc(4096);
  memset(page, 0, 4096);
  if (page == NULL)
  {
    debug_empty("FAILED TO ALLOCATE PAGE TABLE\n");
    return NULL;
  }
  return page;
}

void map_pages(void* v_addr, void* p_addr, uint64_t pages, uint16_t flags)
{
  // printf("Mapping %x to %x\n", v_addr, p_addr);
  for (int j = 0; j < pages; j++)
  {
    if ((uint64_t)v_addr % 4096 != 0)
    {
      return;
    }
    else if ((uint64_t)p_addr % 4096 != 0)
    {
      return;
    }
    uint64_t page_v_addr = (uint64_t)v_addr + (j * 4096);
    uint64_t page_p_addr = (uint64_t)p_addr + (j * 4096);
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

  load_page_table();
}

void copy_existing_pages()
{
  p4_table = new_page_table();
  debug("%x\n", p4_table);
  page_table_t* cr3;
  asm volatile("mov %%cr3, %0"
    : "=r"(cr3));
  for (uint64_t i = 0; i < 512; i++)
  {
    p4_table->pages[i] = cr3->pages[i];
  }
  int res = memcmp(p4_table->pages, cr3->pages, 512);
  if (res != 0)
  {
    debug_empty("Failed to copy pages\n");
    for (;;);
  }
  debug_empty("Copied tables\n");
}

void setup_page_table()
{
  copy_existing_pages();
  load_page_table();
}
