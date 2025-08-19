#include "uefi/uefi.h"
#include "loader.h"
#include "elf.h"
#include <stdbool.h>
bool is_image_valid(Elf64_Ehdr* hdr)
{
  if (hdr->e_ident[EI_MAG0] != 0x7F)
    return false;
  if (hdr->e_ident[EI_MAG1] != 0x45)
    return false;
  if (hdr->e_ident[EI_MAG2] != 0x4C)
    return false;
  if (hdr->e_ident[EI_MAG3] != 0x46)
    return false;
  return true;
}
void* load(char* buf, unsigned int size)
{
  Elf64_Ehdr* hdr = (Elf64_Ehdr*)buf;
  printf("%s, %d, %x, %x, %d, %d\n", hdr->e_ident, hdr->e_type, hdr->e_entry, hdr->e_phoff, hdr->e_phentsize, hdr->e_phnum);
  if (!is_image_valid(hdr))
  {
    return NULL;
  }
  Elf64_Phdr* phdr = (Elf64_Phdr*)(buf + hdr->e_phoff);
  uint32_t align   = 4096;
  uint64_t addr;
  uint64_t begin = 18446744073709551615U;
  uint64_t end   = 0;
  for (uint16_t i = 0; i < hdr->e_phnum; i++)
  {
    Elf64_Phdr current = phdr[i];
    if (current.p_type != PT_LOAD)
    {
      continue;
    }
    if (current.p_align > align)
    {
      align = current.p_align;
    }
    uint64_t phdr_begin = current.p_vaddr & ~(align - 1);
    if (begin > phdr_begin)
    {
      begin = phdr_begin;
    }
    uint64_t phdr_end = (current.p_vaddr + current.p_memsz) & ~(align - 1);
    if (end < phdr_end)
    {
      end = phdr_end;
    }
  }
  uint64_t page_count = (end - begin) / EFI_PAGE_SIZE;
  BS->AllocatePages(AllocateAnyPages, EfiLoaderData, page_count, (void*)&addr);
  printf("Address %x, %d, %x, %x\n", addr, page_count, begin, end);
  map_pages((void*)KERNEL_START, (void*)addr, page_count, 0b10);
  for (int i = 0; i < hdr->e_phnum; i++)
  {
    Elf64_Phdr current = phdr[i];
    printf("%x, %x, %x, %x, %x, %d, %x\n", current.p_align, current.p_filesz, current.p_memsz, current.p_offset, current.p_paddr, current.p_type, current.p_vaddr);
    if (current.p_type != PT_LOAD)
    {
      continue;
    }
    uint64_t p_addr = (uint64_t)addr + current.p_paddr - begin;
    void* res       = memmove((void*)p_addr, buf + current.p_offset, current.p_filesz);
    printf("memmove result %x, p_addr %x\n", res, p_addr);
  }
  void* entry_addr = (void*)addr + hdr->e_entry - begin - KERNEL_START;
  printf("%x\n", entry_addr);
  // if ((uint64_t)entry_addr > (uint64_t)addr + size)
  // {
  //   printf("Failed to get correct address\n");
  //   return NULL;
  // }
  return entry_addr;
}
void* load_kernel()
{
  int kernel_size = 1048576 * 5;
  FILE* elf       = fopen("\\kernel", "r");
  if (elf == NULL)
  {
    return NULL;
  }

  char* elf_buffer;
  BS->AllocatePool(EfiLoaderData, kernel_size, (void**)&elf_buffer);
  if (elf_buffer == NULL)
  {
    return NULL;
  }

  fread(elf_buffer, kernel_size, 1, elf);
  fclose(elf);

  void* ptr = load(elf_buffer, kernel_size);
  BS->FreePool(elf_buffer);
  return ptr;
}
