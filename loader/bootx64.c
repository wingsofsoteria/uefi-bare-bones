#include "uefi/uefi.h"

typedef uint16_t Elf64_Half;
typedef uint32_t Elf64_Word;
typedef int32_t Elf64_Sword;
typedef uint64_t Elf64_Xword;
typedef int64_t Elf64_Sxword;
typedef uint64_t Elf64_Addr;
typedef uint64_t Elf64_Off;
typedef uint16_t Elf64_Section;
typedef Elf64_Half Elf64_Versym;

#define EI_NIDENT (16)
#define EI_MAG0   0
#define EI_MAG1   1
#define EI_MAG2   2
#define EI_MAG3   3
#define PT_LOAD   1

typedef struct
{
  unsigned char e_ident[EI_NIDENT];
  Elf64_Half e_type;
  Elf64_Half e_machine;
  Elf64_Word e_version;
  Elf64_Addr e_entry;
  Elf64_Off e_phoff;
  Elf64_Off e_shoff;
  Elf64_Word e_flags;
  Elf64_Half e_ehsize;
  Elf64_Half e_phentsize;
  Elf64_Half e_phnum;
  Elf64_Half e_shentsize;
  Elf64_Half e_shnum;
  Elf64_Half e_shstrndx;
} Elf64_Ehdr;

typedef struct
{
  Elf64_Word p_type;
  Elf64_Word p_flags;
  Elf64_Off p_offset;
  Elf64_Addr p_vaddr;
  Elf64_Addr p_paddr;
  Elf64_Xword p_filesz;
  Elf64_Xword p_memsz;
  Elf64_Xword p_align;
} Elf64_Phdr;

typedef struct
{
  char magic[5];
  uint64_t base;
  uint64_t size;
  uint32_t pitch;
  uint32_t horizontal_resolution;
  uint32_t vertical_resolution;
  void* initfs;
  int initfs_size;
  efi_memory_descriptor_t* mmap;
  uint64_t mmap_size;
  uint64_t desc_size;
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

int is_image_valid(Elf64_Ehdr* hdr)
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
  uint64_t addr;
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

  BS->AllocatePages(AllocateAnyPages, EfiLoaderData, size / EFI_PAGE_SIZE, (void*)&addr);
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
int
main()
{
  __attribute__((sysv_abi)) int (*ptr)(kernel_bootinfo_t*);
  int kernel_size = 1048576 * 5;
  int initfs_size = 12288;

  FILE* initfs = fopen("\\initfs", "r");
  if (initfs == NULL)
  {
    return 1;
  }
  FILE* elf = fopen("\\kernel", "r");
  if (elf == NULL)
  {
    return 1;
  }

  char* elf_buffer;
  BS->AllocatePool(EfiLoaderData, kernel_size, (void**)&elf_buffer);
  if (elf_buffer == NULL)
  {
    return 1;
  }
  void* initfs_buffer;
  BS->AllocatePool(EfiLoaderData, initfs_size, &initfs_buffer);
  if (initfs_buffer == NULL)
  {
    return 1;
  }
  fread(initfs_buffer, initfs_size, 1, initfs);
  fread(elf_buffer, kernel_size, 1, elf);
  fclose(elf);
  fclose(initfs);

  ptr = load(elf_buffer, kernel_size);
  free(elf_buffer);
  sleep(3);
  efi_guid_t con_guid = EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL_GUID;
  simple_text_output_interface_t* con;
  BS->LocateProtocol(&con_guid, NULL, (void**)&con);
  con->ClearScreen(con);
  if (ptr == NULL)
  {
    printf("Got Null Entry Point\n");
  }

  printf("getting EFI gop\n");
  efi_guid_t gop_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
  efi_gop_t* gop;
  efi_status_t status = BS->LocateProtocol(&gop_guid, NULL, (void**)&gop);
  if (status != EFI_SUCCESS)
  {
    return 1;
  }
  printf("filling out boot info\n");
  kernel_bootinfo_t bootinfo;
  memset(&bootinfo, 0, sizeof(bootinfo));
  strncpy(bootinfo.magic, "OS649", 5);
  bootinfo.base                  = gop->Mode->FrameBufferBase;
  bootinfo.pitch                 = gop->Mode->Information->PixelsPerScanLine * 4;
  bootinfo.horizontal_resolution = gop->Mode->Information->HorizontalResolution;
  bootinfo.vertical_resolution   = gop->Mode->Information->VerticalResolution;
  bootinfo.size                  = gop->Mode->FrameBufferSize;
  bootinfo.initfs                = initfs_buffer;
  bootinfo.initfs_size           = initfs_size;
  printf("filled out boot info\n");
  status             = 0;
  uint64_t mmap_size = 0, desc_size = 0, cnt = 3, map_key = 0;
  uint32_t desc_version;
  efi_memory_descriptor_t* memory_map = NULL;
  while (1)
  {
    status = BS->GetMemoryMap(&mmap_size, memory_map, &map_key, &desc_size, &desc_version);
    if (status == EFI_SUCCESS)
      break;
    status = BS->AllocatePool(EfiLoaderData, mmap_size, (void**)&memory_map);
  }
  status = BS->ExitBootServices(IM, map_key);

  if (status != EFI_SUCCESS || mmap_size == 0)
  {
    for (;;);
  }

  for (int i = 0; i < (mmap_size / desc_size); i++)
  {
    efi_memory_descriptor_t* desc = (efi_memory_descriptor_t*)((uint8_t*)memory_map + (i * desc_size));
    desc->VirtualStart            = desc->PhysicalStart;
  }

  RT->SetVirtualAddressMap(mmap_size, desc_size, desc_version, memory_map);

  bootinfo.mmap      = memory_map;
  bootinfo.mmap_size = mmap_size;
  bootinfo.desc_size = desc_size;

  ptr(&bootinfo);
}
