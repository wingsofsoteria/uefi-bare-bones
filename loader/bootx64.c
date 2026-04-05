#include "uefi/uefi.h"
#include "../kernel/include/types.h"
#include "loader.h"

struct FrameAllocator
{
  mmap_t memory_map;
  loader_memory_descriptor_t* descriptor;
  uint64_t next_frame;
  uint64_t first_frame;
};

uint64_t mmap_index(mmap_t* mmap, uint64_t index)
{
  uint64_t address = (uint64_t)mmap->addr;
  uint64_t offset  = mmap->desc_size * index;

  return address + offset;
}

void mmap_swap(mmap_t* mmap, uint64_t lhs, uint64_t rhs)
{
  if (rhs > (mmap->size / mmap->desc_size))
  {
    for (;;);
  }
  if (lhs > (mmap->size / mmap->desc_size))
  {
    for (;;);
  }

  if (lhs == rhs)
  {
    return;
  }
  uint64_t laddr = (uint64_t)mmap->addr + (lhs * mmap->desc_size);
  uint64_t raddr = (uint64_t)mmap->addr + (rhs * mmap->desc_size);
  char* lptr     = (char*)laddr;
  char* rptr     = (char*)raddr;
  for (int i = 0; i < mmap->desc_size; i++)
  {
    char tmp = lptr[i];
    lptr[i]  = rptr[i];
    rptr[i]  = tmp;
  }
}

uint64_t mmap_partition(mmap_t* mmap, uint64_t low, uint64_t high)
{
  uint64_t pivot = mmap_index(mmap, low + ((high - low) / 2));

  uint64_t lhs = low - 1;
  uint64_t rhs = high + 1;
  while (1)
  {
    while (1)
    {
      lhs++;
      uint64_t lhs_index = mmap_index(mmap, lhs);
      if (lhs_index >= pivot)
      {
        break;
      }
    }
    while (1)
    {
      rhs--;
      uint64_t rhs_index = mmap_index(mmap, rhs);
      if (rhs_index <= pivot)
      {
        break;
      }
    }
    if (lhs >= rhs)
    {
      return rhs;
    }

    mmap_swap(mmap, lhs, rhs);
  }
}

void mmap_sort(mmap_t* mmap, uint64_t low, uint64_t high)
{
  if (low >= high)
  {
    return;
  }
  uint32_t partition = mmap_partition(mmap, low, high);
  mmap_sort(mmap, low, partition);
  mmap_sort(mmap, partition + 1, high);
}

struct FrameAllocator new_frame_allocator()
{
  mmap_t mmap         = quick_memory_map();
  uint64_t mmap_count = (mmap.size / mmap.desc_size);
  mmap_sort(&mmap, 0, mmap_count - 1);
  return (struct FrameAllocator){.memory_map = mmap,
    .descriptor                              = NULL,
    .first_frame                             = 0x100000,
    .next_frame                              = 0x100000};
}

uint64_t max_address(struct FrameAllocator* allocator)
{
  uint64_t max = 0x100000000;
  for (int i = 0;
    i < (allocator->memory_map.size / allocator->memory_map.desc_size); i++)
  {
    loader_memory_descriptor_t* desc =
      (loader_memory_descriptor_t*)((uint64_t)allocator->memory_map.addr +
        (i * allocator->memory_map.desc_size));
    uint64_t size = desc->p_addr + (desc->page_count * EFI_PAGE_SIZE);
    if (size > max)
    {
      max = size;
    }
  }
  return max;
}

uint64_t allocate_page_from_descriptor(struct FrameAllocator* allocator)
{
  if (allocator->descriptor == NULL)
  {
    return 0;
  }

  uint64_t desc_start = allocator->descriptor->p_addr;
  uint64_t desc_end =
    desc_start + (allocator->descriptor->page_count * EFI_PAGE_SIZE);
  if (allocator->next_frame < desc_start)
  {
    allocator->next_frame = desc_start;
  }
  if (allocator->next_frame <= desc_end)
  {
    uint64_t page          = allocator->next_frame;
    allocator->next_frame += EFI_PAGE_SIZE;
    return page;
  }

  return 0;
}

uint64_t allocate_frame(struct FrameAllocator* allocator)
{
  if (allocator->descriptor != NULL)
  {
    uint64_t page = allocate_page_from_descriptor(allocator);
    if (page == 0)
    {
      allocator->descriptor = NULL;
    }
    else
    {
      return page;
    }
  }
  for (int i = 0;
    i < (allocator->memory_map.size / allocator->memory_map.desc_size); i++)
  {
    loader_memory_descriptor_t* desc =
      (loader_memory_descriptor_t*)((uint64_t)allocator->memory_map.addr +
        (i * allocator->memory_map.desc_size));
    if (desc->type != EfiConventionalMemory)
    {
      continue;
    }
    allocator->descriptor = desc;
  }
  return allocate_page_from_descriptor(allocator);
}

uint64_t canonical_address(uint64_t address)
{
  uint64_t canonical = (((int64_t)(address << 16)) >> 16);
  if (canonical != address)
  {
    for (;;);
  }
  return canonical;
}

void setup_page_table_new(struct FrameAllocator* allocator,
  uint64_t frame_buffer_addr, uint64_t frame_buffer_len)
{
  page_table_t* old_table;
  asm volatile("mov %%cr3, %0"
    : "=r"(old_table));

  uint64_t address = allocate_frame(allocator);
  if (address == 0)
  {
    for (;;);
  }
  uint64_t max            = canonical_address(max_address(allocator));
  uint64_t p4_index       = ((max >> 12) >> 27) - 1;
  page_table_t* new_table = (void*)address;
  __builtin_memset(new_table, 0, sizeof(page_table_t));

  for (int i = 0; i <= p4_index; i++)
  {
    new_table->pages[i] = old_table->pages[i];
  }
  uint64_t fb_start = canonical_address(frame_buffer_addr);
  uint64_t fb_end   = fb_start + frame_buffer_len;
  uint64_t p4_start = (fb_start >> 12) >> 27;
  uint64_t p4_end   = (fb_end >> 12) >> 27;
  for (int i = p4_start; i <= p4_end; i++)
  {
    new_table->pages[i] = old_table->pages[i];
  }
  asm volatile("mov %0, %%cr3"
    :
    : "r"((uint64_t)new_table));

  uint64_t kernel_frame = allocate_frame(allocator);
  if (kernel_frame == 0)
  {
    for (;;);
  }
  page_table_t* kernel_page_table = (void*)kernel_frame;
  __builtin_memset(kernel_page_table, 0, sizeof(page_table_t));
}

void kernel_map()
{
}

int main()
{
  /*setup_page_table();
  __attribute__((sysv_abi)) int (*ptr)(kernel_bootinfo_t*, void*);

  ptr = load_kernel();
  if (ptr == NULL)
  {
    printf("Got Null Entry Point\n");
    for (;;);
  }
  kernel_bootinfo_t* bootinfo = get_bootinfo();

  efi_status_t status = BS->ExitBootServices(IM, bootinfo->mmap->key);

  if (status != EFI_SUCCESS)
  {
    for (;;);
  }

  load_page_table();
  ptr(bootinfo, base_address);
*/
  return 1;
}
