#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#define EI_NIDENT 16
typedef uint16_t Elf64_Half;
typedef uint32_t Elf64_Word;
typedef uint64_t Elf64_Addr;
typedef uint64_t Elf64_Off;
typedef uint64_t Elf64_Xword;
typedef struct {
        unsigned char   e_ident[EI_NIDENT];
        Elf64_Half      e_type;
        Elf64_Half      e_machine;
        Elf64_Word      e_version;
        Elf64_Addr      e_entry;
        Elf64_Off       e_phoff;
        Elf64_Off       e_shoff;
        Elf64_Word      e_flags;
        Elf64_Half      e_ehsize;
        Elf64_Half      e_phentsize;
        Elf64_Half      e_phnum;
        Elf64_Half      e_shentsize;
        Elf64_Half      e_shnum;
        Elf64_Half      e_shstrndx;
} Elf64_Ehdr;
typedef struct {
	Elf64_Word	p_type;
	Elf64_Word	p_flags;
	Elf64_Off	p_offset;
	Elf64_Addr	p_vaddr;
	Elf64_Addr	p_paddr;
	Elf64_Xword	p_filesz;
	Elf64_Xword	p_memsz;
	Elf64_Xword	p_align;
} Elf64_Phdr;

int check_elf_header(Elf64_Ehdr* header) {
    printf("ident %s\n", header->e_ident);
    if (!header) return 0;
    if (header->e_ident[0] != 0x7F || header->e_ident[1] != 0x45 || header->e_ident[2] != 0x4C || header->e_ident[3] != 0x46) {
        return 0;
    }
    return 1;
}

void loadelf(Elf64_Ehdr header, void* bytes) {
    printf("Offset %d\nEntry Size %d\nNum Entries %d", header.e_phoff, header.e_phentsize, header.e_phnum);
    for (uint16_t i = 0; i < header.e_phnum; i++) {
        size_t offset = header.e_phoff + i * header.e_phentsize;
        Elf64_Phdr pheader;
        memmove(&pheader, bytes + offset, sizeof(Elf64_Phdr));
        
    }
}

void* readelf(char* filename) {
    FILE* exec = fopen(filename, "rb");
    if (exec == NULL) {
        perror("fopen error");
        return NULL;
    }
    if (fseek(exec, 0, SEEK_END)) {
        perror("FSEEK Error");
        return NULL;
    }
    long size = ftell(exec);
    fseek(exec, 0, SEEK_SET);
    char* bytes = calloc(size, sizeof(char));
    fread(bytes, sizeof(char), size, exec);
    fclose(exec);
    Elf64_Ehdr header;
    memmove(&header, bytes, sizeof(Elf64_Ehdr));
    if (!check_elf_header(&header)) {
        printf("invalid header found: %s", header.e_ident);
        return NULL;
    }
    printf("%ld\n", size);
    printf("0x%x 0x%x\n", header.e_type, bytes[16]);
    printf("0x%x 0x%x\n", header.e_machine, bytes[18]);
    printf("0x%x 0x%x\n", header.e_version, bytes[20]);
    printf("0x%x 0x%x, 0x%x\n", header.e_entry, bytes[25], bytes[26]);
    printf("0x%x 0x%x\n", header.e_phoff, bytes[32]);
    printf("0x%x 0x%x\n", header.e_flags, 0);
    printf("0x%x 0x%x\n", header.e_ehsize, bytes[52]);
    printf("0x%x 0x%x\n", header.e_phentsize, bytes[54]);
    printf("0x%x 0x%x\n", header.e_phnum, bytes[56]);
    printf("0x%x 0x%x\n", header.e_shentsize, bytes[58]);
    printf("0x%x 0x%x\n", header.e_shnum, bytes[60]);
    printf("0x%x 0x%x\n", header.e_shstrndx, bytes[62]);



    // loadelf(header, bytes);
    return &header;
}

int main(int argc, char** argv) {
    Elf64_Ehdr* header = readelf("./test.elf");
    return 0;
}