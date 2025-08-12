#include <elf.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
typedef struct
{
  char magic[5];
  uint64_t base;
  uint32_t pitch;
  uint32_t horizontal_resolution;
  uint32_t vertical_resolution;
  void* initfs;
} kernel_bootinfo_t;

typedef struct
{
  uint16_t c_magic;
  uint16_t c_dev;
  uint16_t c_ino;
  uint16_t c_mode;
  uint16_t c_uid;
  uint16_t c_gid;
  uint16_t c_nlink;
  uint16_t c_rdev;
  uint16_t c_mtime[2];
  uint16_t c_namesize;
  uint16_t c_filesize[2];
} cpio_header_t;

int
is_image_valid(Elf64_Ehdr* hdr)
{
  if (hdr->e_ident[EI_MAG0] != 0x7F)
    return 0;
  if (hdr->e_ident[EI_MAG1] != 0x45)
    return 0;
  if (hdr->e_ident[EI_MAG2] != 0x4C)
    return 0;
  if (hdr->e_ident[EI_MAG3] != 0x46)
    return 0;
  return 1;
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
  void* addr;
  uint64_t begin = 18446744073709551615;
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
  }

  addr = mmap(NULL, size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  printf("Address %x\n", addr);
  for (int i = 0; i < hdr->e_phnum; i++)
  {
    Elf64_Phdr current = phdr[i];
    printf("%x, %x, %x, %x, %x, %d, %x\n", current.p_align, current.p_filesz, current.p_memsz, current.p_offset, current.p_paddr, current.p_type, current.p_vaddr);
    if (current.p_type != PT_LOAD)
    {
      continue;
    }
    uint64_t p_addr = (uint64_t)addr + current.p_vaddr - begin;
    void* res       = memmove((void*)p_addr, buf + current.p_offset, current.p_filesz);
    printf("memmove result %x, p_addr %x\n", res, p_addr);
  }
  void* entry_addr = (void*)addr + hdr->e_entry - begin;
  printf("%x\n", entry_addr);
  if ((uint64_t)entry_addr > (uint64_t)addr + size)
  {
    printf("Failed to get correct address\n");
    return NULL;
  }
  return entry_addr;
}

int main()
{
  // printf("Hello World");
  // for (;;);
  char* buf = calloc(1280, 4096);
  if (buf == NULL)
  {
    printf("Could not allocate buffer");
    for (;;);
  }
  FILE* elf = fopen("test", "r");

  if (elf == NULL)
  {
    printf("Failed to open kernel");
    for (;;);
  }
  __attribute__((sysv_abi)) int (*ptr)(kernel_bootinfo_t*);
  fread(buf, 5242880, 1, elf);
  ptr = load(buf, 5242880);
  fclose(elf);
  free(buf);

  FILE* cpio = fopen("../initfs", "r");
  void* mem  = mmap(NULL, 8192, PROT_READ, MAP_PRIVATE, fileno(cpio), 0);
  kernel_bootinfo_t bootinfo;
  memset(&bootinfo, 0, sizeof(bootinfo));
  strncpy(bootinfo.magic, "TEST5", 5);
  bootinfo.base                  = 0x80000000;
  bootinfo.pitch                 = 1280 * 4;
  bootinfo.horizontal_resolution = 1920;
  bootinfo.vertical_resolution   = 1080;
  bootinfo.initfs                = mem;
  cpio_header_t* hdr             = (cpio_header_t*)mem;
  if (hdr->c_magic != 0070707)
  {
    return 1;
  }
  int hdr_size = sizeof(cpio_header_t);
  char* font   = (char*)(mem) + hdr_size + hdr->c_namesize + 1;
  printf("\n\n test %.2X %.2X %.2X", font[33 * 8], font[34 * 8], font[35 * 8]);
  if (ptr == NULL)
  {
    return -1;
  }
  return ptr(&bootinfo);
}
