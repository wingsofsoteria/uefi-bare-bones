#include "uefi/uefi.h"
#include <stdint.h>
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

Elf64_Phdr* phdrs;

int check_elf_header(Elf64_Ehdr* header) {
    if (!header) return 0;
    if (header->e_ident[0] != 0x7F || header->e_ident[1] != 0x45 || header->e_ident[2] != 0x4C || header->e_ident[3] != 0x46) {
        return 0;
    }
    return 1;
}

void loadelf(Elf64_Ehdr* header, void* bytes) {
    //read basic data
    printf("Offset %d\nEntry Size %d\nNum Entries %d\n", header->e_phoff, header->e_phentsize, header->e_phnum);
    phdrs = calloc(header->e_phnum, header->e_phentsize);
    //stuff for program headers
    uint64_t align = EFI_PAGE_SIZE;
    uint64_t begin = 18446744073709551615;
    uint64_t end = 0;
    //read some data from the program headers to get entry data
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
        uint64_t phdr_end = (phdr.p_vaddr + phdr.p_memsz + align - 1) & ~(align - 1);
        if (end < phdr_end) {
            end = phdr_end;
        }
    }
    uint64_t size = end - begin;
    printf("%u %u %u\n", size, begin, end);
    uint64_t addr;
    //allocate memory for program headers
    ST->BootServices->AllocatePages(AllocateAnyPages, EfiLoaderData, size / 4096, &addr);
    memset((void *)addr, 0, size);
    for (int i = 0; i < header->e_phnum; i++) {
        Elf64_Phdr phdr = phdrs[i];
        if (phdr.p_type != 1)
			continue;
        uint64_t p_addr = addr + phdr.p_vaddr - begin;
        //copy header into p_addr
        memmove((void*)p_addr, bytes + phdr.p_offset, phdr.p_filesz);
    }
    printf("done loading\n");
    uint64_t entry_addr = addr + header->e_entry - begin;
    int(*entry)(void) = (int (*)(void))entry_addr;
    int test = (*entry)();
    printf("test value %d", test);
}

void print_header(Elf64_Ehdr* header, char* bytes) {
    for (int i=0; i < 4; i++) {
        printf("%x ", bytes[0 + (i*16)]);
        printf("%x ", bytes[1 + (i*16)]);
        printf("%x ", bytes[2 + (i*16)]);
        printf("%x ", bytes[3 + (i*16)]);
        printf("%x ", bytes[4 + (i*16)]);
        printf("%x ", bytes[5 + (i*16)]);
        printf("%x ", bytes[6 + (i*16)]);
        printf("%x ", bytes[7 + (i*16)]);
        printf("%x ", bytes[8 + (i*16)]);
        printf("%x ", bytes[9 + (i*16)]);
        printf("%x ", bytes[10 + (i*16)]);
        printf("%x ", bytes[11 + (i*16)]);
        printf("%x ", bytes[12 + (i*16)]);
        printf("%x ", bytes[13 + (i*16)]);
        printf("%x ", bytes[14 + (i*16)]);
        printf("%x \n", bytes[15 + (i*16)]);
    }
    printf("0x%x 0x%x\n", header->e_type, bytes[16]);
    printf("0x%x 0x%x\n", header->e_machine, bytes[18]);
    printf("0x%x 0x%x\n", header->e_version, bytes[20]);
    printf("0x%x 0x%x, 0x%x\n", header->e_entry, bytes[25], bytes[26]);
    printf("0x%x 0x%x\n", header->e_phoff, bytes[32]);
    printf("0x%x 0x%x\n", header->e_flags, 0);
    printf("0x%x 0x%x\n", header->e_ehsize, bytes[52]);
    printf("0x%x 0x%x\n", header->e_phentsize, bytes[54]);
    printf("0x%x 0x%x\n", header->e_phnum, bytes[56]);
    printf("0x%x 0x%x\n", header->e_shentsize, bytes[58]);
    printf("0x%x 0x%x\n", header->e_shnum, bytes[60]);
    printf("0x%x 0x%x\n", header->e_shstrndx, bytes[62]);
}

void* readelf(char* filename) {
    
    printf("\nreading elf file\n");
    FILE* exec = fopen(filename, CL("r"));
    if (!exec) {
        printf("failed to open file %s %d", filename, errno);
        return NULL;
    }if (fseek(exec, 0, SEEK_END)) {
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
    loadelf(&header, bytes);
    
    return &header;
}

int main(int argc, char** argv) {
    printf("hello!");
    Elf64_Ehdr* header = readelf("\\test.elf");
    // DIR* root = opendir("\\");
    // if (root == NULL) {
    //     printf("BIG ERROR %d", errno);
    // }
    // printf("open dir\n");
    // struct dirent* dirents = readdir(root);
    // printf("readdir\n");
    // printf("dir entry %s", dirents[0].d_name);

    for(;;);
    return 0;
}