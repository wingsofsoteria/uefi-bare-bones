// #include <stdint.h>

typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#define EI_NIDENT 16
typedef uint16_t Elf64_Half;
typedef uint32_t Elf64_Word;
typedef uint64_t Elf64_Addr;
typedef uint64_t Elf64_Off;
typedef uint64_t Elf64_Xword;
typedef struct {
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
typedef struct {
  Elf64_Word p_type;
  Elf64_Word p_flags;
  Elf64_Off p_offset;
  Elf64_Addr p_vaddr;
  Elf64_Addr p_paddr;
  Elf64_Xword p_filesz;
  Elf64_Xword p_memsz;
  Elf64_Xword p_align;
} Elf64_Phdr;

Elf64_Phdr* phdrs;

int check_elf_header(Elf64_Ehdr* header) {
  printf("ident %s\n", header->e_ident);
  if (!header) return 0;
  if (header->e_ident[0] != 0x7F || header->e_ident[1] != 0x45 ||
      header->e_ident[2] != 0x4C || header->e_ident[3] != 0x46) {
    return 0;
  }
  return 1;
}

void* loadelf(Elf64_Ehdr* header, void* bytes) {
  // read basic data
  printf("Offset %lu\nEntry Size %d\nNum Entries %d\n", header->e_phoff,
         header->e_phentsize, header->e_phnum);
  phdrs = calloc(header->e_phnum, header->e_phentsize);
  // stuff for program headers
  uint64_t align = 4096;
  uint64_t begin = 18446744073709551615;
  uint64_t end = 0;
  // read some data from the program headers to get entry data
  for (uint16_t i = 0; i < header->e_phnum; i++) {
    size_t offset = header->e_phoff + i * header->e_phentsize;
    memmove(&(phdrs[i]), bytes + offset, sizeof(Elf64_Phdr));
    Elf64_Phdr phdr = phdrs[i];
    if (phdr.p_type != 1) {
      continue;
    }
    if (phdr.p_align > align) {
      align = phdr.p_align;
    }
    uint64_t phdr_begin = phdr.p_vaddr & ~(align - 1);
    if (begin > phdr_begin) {
      begin = phdr_begin;
    }
    uint64_t phdr_end =
        (phdr.p_vaddr + phdr.p_memsz + align - 1) & ~(align - 1);
    if (end < phdr_end) {
      end = phdr_end;
    }
  }
  uint64_t size = end - begin;
  printf("%lu %lu %lu\n", size, begin, end);
  void* addr;
  // allocate memory for program headers
  addr = mmap(NULL, size, PROT_READ | PROT_WRITE | PROT_EXEC,
              MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
  memset(addr, 0, size);
  for (int i = 0; i < header->e_phnum; i++) {
    Elf64_Phdr phdr = phdrs[i];
    if (phdr.p_type != 1) continue;
    void* p_addr = addr + phdr.p_vaddr - begin;
    // copy header into p_addr
    memmove(p_addr, bytes + phdr.p_offset, phdr.p_filesz);
  }
  printf("done loading\n");
  void* entry_addr = addr + header->e_entry - begin;
  return entry_addr;
}

void* readelf(char* filename) {
  printf("\nreading elf file\n");
  FILE* exec = fopen(filename, "r");
  if (!exec) {
    return NULL;
  }
  if (fseek(exec, 0, SEEK_END)) {
    return NULL;
  }
  long size = ftell(exec);
  fseek(exec, 0, SEEK_SET);
  char* bytes = calloc(size, sizeof(char));
  fread(bytes, sizeof(char), size, exec);
  fclose(exec);
  printf("%x, %x, %x, %x\n", bytes[0], bytes[1], bytes[2], bytes[3]);
  Elf64_Ehdr header;
  memmove(&header, bytes, sizeof(Elf64_Ehdr));
  if (!check_elf_header(&header)) {
    printf("invalid elf header");
    return NULL;
  }
  return loadelf(&header, bytes);
}

int main(int argc, char** argv) {
  void* addr = readelf(argv[1] == NULL ? "./test.elf" : argv[1]);
  uint64_t (*test)(uint64_t) = (uint64_t (*)(uint64_t))addr;
  int test_result = (*test)(1);
  printf("%d", test_result);
  return 0;
}
