#include "test.h"
typedef unsigned int uint32_t;
typedef unsigned short int uint16_t;
typedef unsigned char uint8_t;
typedef unsigned long long int uint64_t;
#define EI_NIDENT (16)

/* Type for a 16-bit quantity.  */
typedef uint16_t Elf64_Half;

/* Types for signed and unsigned 32-bit quantities.  */
typedef uint32_t Elf64_Word;

/* Types for signed and unsigned 64-bit quantities.  */
typedef uint64_t Elf64_Xword;

/* Type of addresses.  */
typedef uint64_t Elf64_Addr;

/* Type of file offsets.  */
typedef uint64_t Elf64_Off;

/* Type for section indices, which are 16-bit quantities.  */
typedef uint16_t Elf64_Section;

/* Type for version symbol information.  */
typedef Elf64_Half Elf64_Versym;
#define EI_MAG0 0    /* File identification byte 0 index */
#define ELFMAG0 0x7f /* Magic number byte 0 */

#define EI_MAG1 1   /* File identification byte 1 index */
#define ELFMAG1 'E' /* Magic number byte 1 */

#define EI_MAG2 2   /* File identification byte 2 index */
#define ELFMAG2 'L' /* Magic number byte 2 */

#define EI_MAG3 3   /* File identification byte 3 index */
#define ELFMAG3 'F' /* Magic number byte 3 */

typedef struct
{
  unsigned char e_ident[EI_NIDENT]; /* Magic number and other info */
  Elf64_Half e_type;                /* Object file type */
  Elf64_Half e_machine;             /* Architecture */
  Elf64_Word e_version;             /* Object file version */
  Elf64_Addr e_entry;               /* Entry point virtual address */
  Elf64_Off e_phoff;                /* Program header table file offset */
  Elf64_Off e_shoff;                /* Section header table file offset */
  Elf64_Word e_flags;               /* Processor-specific flags */
  Elf64_Half e_ehsize;              /* ELF header size in bytes */
  Elf64_Half e_phentsize;           /* Program header table entry size */
  Elf64_Half e_phnum;               /* Program header table entry count */
  Elf64_Half e_shentsize;           /* Section header table entry size */
  Elf64_Half e_shnum;               /* Section header table entry count */
  Elf64_Half e_shstrndx;            /* Section header string table index */
} Elf64_Ehdr;
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

int
_start(kernel_bootinfo_t* bootinfo)
{
  cpio_header_t* hdr = (cpio_header_t*)bootinfo->initfs;
  if (hdr->c_magic != 0070707)
  {
    return 1;
  }
  int hdr_size     = sizeof(cpio_header_t);
  Elf64_Ehdr* ehdr = (Elf64_Ehdr*)(hdr + hdr_size + hdr->c_namesize + 2);
  if (ehdr->e_ident[EI_MAG0] != 0x7F)
    return 2;
  if (ehdr->e_ident[EI_MAG1] != 0x45)
    return 3;
  if (ehdr->e_ident[EI_MAG2] != 0x4C)
    return 4;
  if (ehdr->e_ident[EI_MAG3] != 0x46)
    return 5;

  return 0;
}
