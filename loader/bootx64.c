#include "uefi/uefi.h"
#include <elf.h>

int is_image_valid(Elf64_Ehdr* hdr) {
  if (hdr->e_ident[EI_MAG0] != 0x7F) return 0;
  if (hdr->e_ident[EI_MAG1] != 0x45) return 0;
  if (hdr->e_ident[EI_MAG2] != 0x4C) return 0;
  if (hdr->e_ident[EI_MAG3] != 0x46) return 0;
  return 1;
}

void* load(char* buf, unsigned int size) {
  Elf64_Ehdr* hdr = (Elf64_Ehdr*)buf;
  printf("%s, %d, %x, %x, %d, %d\n", hdr->e_ident, hdr->e_type, hdr->e_entry,
         hdr->e_phoff, hdr->e_phentsize, hdr->e_phnum);
  if (!is_image_valid(hdr)) {
    return NULL;
  }
  Elf64_Phdr* phdr = (Elf64_Phdr*)(buf + hdr->e_phoff);
  uint32_t align = 4096;
  void* addr;
  uint64_t begin = 18446744073709551615;
  for (uint16_t i = 0; i < hdr->e_phnum; i++) {
    Elf64_Phdr current = phdr[i];
    if (current.p_type != PT_LOAD) {
      continue;
    }
    if (current.p_align > align) {
      align = current.p_align;
    }
    uint64_t phdr_begin = current.p_vaddr & ~(align - 1);
    if (begin > phdr_begin) {
      begin = phdr_begin;
    }
  }

  BS->AllocatePool(EfiLoaderData, size / EFI_PAGE_SIZE, (void**)&addr);
  printf("Address %x\n", addr);
  for (int i = 0; i < hdr->e_phnum; i++) {
    Elf64_Phdr current = phdr[i];
    printf("%x, %x, %x, %x, %x, %d, %x\n", current.p_align, current.p_filesz,
           current.p_memsz, current.p_offset, current.p_paddr, current.p_type,
           current.p_vaddr);
    if (current.p_type != PT_LOAD) {
      continue;
    }
    uint64_t p_addr = (uint64_t)addr + current.p_vaddr - begin;
    void* res =
        memmove((void*)p_addr, buf + current.p_offset, current.p_filesz);
    printf("memmove result %x, p_addr %x\n", res, p_addr);
  }
  void* entry_addr = (void*)addr + hdr->e_entry - begin;
  printf("%x", entry_addr);
  if ((uint64_t)entry_addr > (uint64_t)addr + size) {
    printf("Failed to get correct address\n");
    return NULL;
  }
  return entry_addr;
}
int main() {
  // printf("Hello World");
  // for (;;);
  char* buf = calloc(1280, 4096);
  if (buf == NULL) {
    printf("Could not allocate buffer");
    for (;;);
  }
  FILE* elf = fopen("\\kernel", "r");

  if (elf == NULL) {
    printf("Failed to open kernel");
    for (;;);
  }
  __attribute__((sysv_abi)) int (*ptr)(uint64_t, uint32_t, uint32_t, uint32_t,
                                       char[5]);
  int read = fread(buf, 5242880, 1, elf);
  ptr = load(buf, 5242880);

  if (ptr != NULL) {
    printf("Loading Successful\n");
    efi_guid_t gop_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    efi_gop_t* gop;
    BS->LocateProtocol(&gop_guid, NULL, (void**)&gop);
    printf("%x, %d, %d, %d", gop->Mode->FrameBufferBase,
           gop->Mode->Information->PixelsPerScanLine * 4,
           gop->Mode->Information->HorizontalResolution,
           gop->Mode->Information->VerticalResolution);
    printf("%x\n", ptr(gop->Mode->FrameBufferBase,
                       gop->Mode->Information->HorizontalResolution,
                       gop->Mode->Information->VerticalResolution,
                       gop->Mode->Information->PixelsPerScanLine * 4, "OS577"));
  } else {
    printf("Loading unsuccessful\n");
  }
  fclose(elf);
  for (;;);
}
