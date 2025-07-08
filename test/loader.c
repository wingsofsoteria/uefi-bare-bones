#include <elf.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

int is_image_valid(Elf64_Ehdr* hdr) {
  if (hdr->e_ident[EI_MAG0] != 0x7F) return 0;
  if (hdr->e_ident[EI_MAG1] != 0x45) return 0;
  if (hdr->e_ident[EI_MAG2] != 0x4C) return 0;
  if (hdr->e_ident[EI_MAG3] != 0x46) return 0;
  return 1;
}

void* load(char* buf, unsigned int size) {
  Elf64_Ehdr* hdr = (Elf64_Ehdr*)buf;
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
  addr = mmap(NULL, size, PROT_READ | PROT_WRITE | PROT_EXEC,
              MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  for (int i = 0; i < hdr->e_phnum; i++) {
    Elf64_Phdr current = phdr[i];
    if (current.p_type != PT_LOAD) {
      continue;
    }
    uint64_t p_addr = (uint64_t)addr + current.p_vaddr - begin;
    memmove((void*)p_addr, buf + current.p_offset, current.p_filesz);
    // if (!(current.p_flags & PF_W)) {
    //   mprotect((void*)p_addr, current.p_filesz, PROT_READ);
    // }
    // if (current.p_flags & PF_X) {
    //   mprotect((void*)p_addr, current.p_filesz, PROT_EXEC);
    // }
  }
  void* entry_addr = addr + hdr->e_entry - begin;
  if ((uint64_t)entry_addr > (uint64_t)addr + size) {
    printf("Failed to get correct address\n");
    return NULL;
  }
  return entry_addr;
}
int main(int argc, char** argv) {
  char buf[5242880];
  memset(buf, 0, sizeof(buf));
  FILE* elf = fopen(argv[1], "rb");

  if (elf == NULL) {
    return 1;
  }
  int (*ptr)(char[5]);

  fread(buf, sizeof(buf), 1, elf);
  ptr = load(buf, sizeof(buf));

  if (ptr != NULL) {
    printf("Loading Successful\n");
    printf("%d\n", ptr("TEST5"));
  } else {
    printf("Loading unsuccessful\n");
  }
  fclose(elf);
  return 0;
}
