#include "memory/paging.h"

#include "initial_frame_allocator.h"
#include "log.h"
#include "paging_internal.h"
#include "types.h"
#include "utils.h"
static page_table_t* page_table = NULL;
static int           debug      = 0;

uint64_t virtual_to_physical(uint64_t virtual)
{
  uint16_t p4_index = (virtual >> 12 >> 9 >> 9 >> 9) & 0x1FF;
  uint16_t p3_index = (virtual >> 12 >> 9 >> 9) & 0x1FF;
  uint16_t p2_index = (virtual >> 12 >> 9) & 0x1FF;
  uint16_t p1_index = (virtual >> 12) & 0x1FF;

  uint64_t p3_physical = page_table->pages[p4_index].address << 12;
  if (p3_physical == 0)
    {
      kernel_log_error(
        "Could not get level 3 page table, %lu is not mapped to "
        "any physical address",
        virtual
      );
      return 0;
    }
  page_table_t* p3          = (void*)(p3_physical + hhdm_mapping);
  uint64_t      p2_physical = p3->pages[p3_index].address << 12;
  if (p2_physical == 0)
    {
      kernel_log_error(
        "Could not get level 2 page table, %lu is not mapped to "
        "any physical address",
        virtual
      );
      return 0;
    }
  page_table_t* p2          = (void*)(p2_physical + hhdm_mapping);
  uint64_t      p1_physical = p2->pages[p2_index].address << 12;
  if (p1_physical == 0)
    {
      kernel_log_error(
        "Could not get level 1 page table, %lu is not mapped to "
        "any physical address",
        virtual
      );
      return 0;
    }
  page_table_t* p1         = (void*)(p1_physical + hhdm_mapping);
  uint64_t      page_entry = p1->pages[p1_index].address << 12;
  if (page_entry == 0)
    {
      kernel_log_error(
        "Could not get page entry, %llu is not mapped to any physical address",
        virtual
      );
      return 0;
    }
  // kernel_log_debug("%x is mapped to %x", virtual, page_entry);
  return page_entry;
}

static void set_entry(page_entry_t* entry, uint64_t address, uint16_t flags)
{
  entry->present         = flags & 0x1;
  entry->writable        = (flags & 0x2) >> 1;
  entry->user_accessable = (flags & 0x4) >> 2;
  entry->write_through   = (flags & 0x8) >> 3;
  entry->cache_disable   = (flags & 0x10) >> 4;
  entry->accessed        = (flags & 0x20) >> 5;
  entry->unused_1        = 0;
  entry->page_size       = (flags & 0x40) >> 6;
  entry->unused_2        = 0;
  entry->address         = (address >> 12);
  entry->unused_3        = 0;
  entry->execute_disable = (flags & 0x80) >> 7;
}

static void __allocate_entry(
  page_table_t* table,
  uint16_t      index,
  uint16_t      flags
)
{
  uint64_t address = allocate_frame();
  if (address == 0) { abort_msg("Failed to allocate space for page entry"); }
  set_entry(&table->pages[index], address, flags);
}

void map_page(uint64_t page, uint64_t frame, uint16_t flags)
{
  uint16_t p4_index = (page >> 12 >> 9 >> 9 >> 9) & 0x1FF;
  uint16_t p3_index = (page >> 12 >> 9 >> 9) & 0x1FF;
  uint16_t p2_index = (page >> 12 >> 9) & 0x1FF;
  uint16_t p1_index = (page >> 12) & 0x1FF;

  if (page_table->pages[p4_index].present == 0)
    {
      __allocate_entry(page_table, p4_index, flags);
    }
  page_table_t* p3 =
    (void*)((page_table->pages[p4_index].address << 12) + hhdm_mapping);
  if (p3->pages[p3_index].present == 0)
    {
      __allocate_entry(p3, p3_index, flags);
    }
  page_table_t* p2 =
    (void*)((p3->pages[p3_index].address << 12) + hhdm_mapping);
  if (p2->pages[p2_index].present == 0)
    {
      __allocate_entry(p2, p2_index, flags);
    }
  page_table_t* p1 =
    (void*)((p2->pages[p2_index].address << 12) + hhdm_mapping);
  if (debug)
    {
      kernel_log_debug(
        "Mapping address %lu to %lu\n\tP4[%i]: %p\n\tP3[%i]: "
        "%p\n\tP2[%i]: %p\n\tEntry: %i",
        frame,
        page,
        p4_index,
        p3,
        p3_index,
        p2,
        p2_index,
        p1,
        p1_index
      );
    }
  if (p1->pages[p1_index].present)
    {
      abort_msg("Page is already mapped %p -> %p", page, p1->pages[p1_index]);
      return;
    }
  set_entry(&p1->pages[p1_index], frame, flags);
  flush_tlb(page);
}

void unmap_page(uint64_t page)
{
  uint16_t p4_index = (page >> 12 >> 9 >> 9 >> 9) & 0x1FF;
  uint16_t p3_index = (page >> 12 >> 9 >> 9) & 0x1FF;
  uint16_t p2_index = (page >> 12 >> 9) & 0x1FF;
  uint16_t p1_index = (page >> 12) & 0x1FF;
  if (page_table->pages[p4_index].present == 0) { return; }
  page_table_t* p3 =
    (void*)((page_table->pages[p4_index].address << 12) + hhdm_mapping);
  if (p3->pages[p3_index].present == 0) { return; }
  page_table_t* p2 =
    (void*)((p3->pages[p3_index].address << 12) + hhdm_mapping);
  if (p2->pages[p2_index].present == 0) { return; }
  page_table_t* p1 =
    (void*)((p2->pages[p2_index].address << 12) + hhdm_mapping);
  if (debug)
    {
      kernel_log_debug(
        "Unmapping page %lu\n\tP4[%i]: %p\n\tP3[%i]: "
        "%p\n\tP2[%i]: %p\n\tEntry: %i",
        page,
        p4_index,
        p3,
        p3_index,
        p2,
        p2_index,
        p1,
        p1_index
      );
    }
  p1->pages[p1_index] = (page_entry_t){ 0 };
  flush_tlb(page);
}

void init_page_table()
{ page_table = (void*)(((read_cr3() << 12) >> 12) + hhdm_mapping); }
