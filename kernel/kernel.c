#include <graphics.h>
#include <kernel.h>
#include <stdint.h>
#include <stdio.h>

// TODO FIGURE OUT WHY EXCEPTIONS AREN'T BEING HANDLED
// TODO FIX ISSUE WITH FRAME BUFFER REQUESTING RESTRICTED PAGES
// DONE: IDENTITY MAP PAGES
// UEFI ENABLES PAGING SO SKIP THAT
// TODO TRANSLATE THE EFI MEMORY DESCRIPTORS TO SOMETHING THAT WE CAN USE
// TODO ENABLE ACPI IN THE LOADER AND ADD RSDP TO OUR BOOTINFO
typedef struct
{
  uint32_t type;
  uint32_t __pad;
  uint64_t p_addr;
  uint64_t v_addr;
  uint64_t page_count;
  uint64_t __attrib;
} efi_memory_descriptor_t;
typedef struct
{
  char magic[5];
  uint64_t base;
  uint64_t size;
  uint32_t pitch;
  uint32_t horizontal_resolution;
  uint32_t vertical_resolution;
  void* ptr_initfs;
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

struct idt_entry_t
{
  uint16_t address_low;
  uint16_t selector;
  uint8_t ist;
  uint8_t flags;
  uint16_t address_mid;
  uint32_t address_high;
  uint32_t reserved;
} __attribute__((packed));

struct idt_entry_t idt[256];
struct idtr_t
{
  uint16_t limit;
  uint64_t base;
} __attribute__((packed));

void load_idt(void* idt_addr)
{
  struct idtr_t idt_reg;
  idt_reg.limit = 0xFFF;
  idt_reg.base  = (uint64_t)idt_addr;
  asm volatile("lidt %0" ::"m"(idt_reg));
}
void set_idt_entry(uint8_t vector, void* handler, uint8_t dpl)
{
  uint64_t handler_addr = (uint64_t)handler;

  struct idt_entry_t* entry = &idt[vector];
  entry->address_low        = handler_addr & 0xFFFF;
  entry->address_mid        = (handler_addr >> 16) & 0xFFFF;
  entry->address_high       = handler_addr >> 32;
  // your code selector may be different!
  entry->selector = 0x8;
  // trap gate + present + DPL
  entry->flags = 0b1110 | ((dpl & 0b11) << 5) | (1 << 7);
  // ist disabled
  entry->ist = 0;
}

void pf_handler()
{
  qemu_print("Kernel Encountered Page Fault");
  asm volatile("cli; hlt");
}

int _start(kernel_bootinfo_t* bootinfo)
{
  qemu_print("Kernel Start\n");
  if (bootinfo->mmap_size == 0)
    return 1;
  efi_memory_descriptor_t* mmap = bootinfo->mmap;

  qemu_printn((uint64_t)mmap, 16);
  qemu_print(" ");
  qemu_printn(bootinfo->mmap_size, 10);
  qemu_print(" ");
  qemu_printn(bootinfo->desc_size, 10);
  qemu_print("\n");
  uint64_t page_num = 0;
  for (int i = 0; i < (bootinfo->mmap_size / bootinfo->desc_size); i++)
  {
    efi_memory_descriptor_t* desc = (efi_memory_descriptor_t*)((uint8_t*)mmap + (i * bootinfo->desc_size));
    uint64_t paddr                = desc->p_addr;
    uint64_t vaddr                = desc->v_addr;
    qemu_print("Entry ");
    qemu_printn(i, 10);
    qemu_print(": Type ");
    qemu_printn(desc->type, 10);
    qemu_print(", PADDR ");
    qemu_printn(desc->p_addr, 16);
    qemu_print(", VADDR");
    qemu_printn(desc->v_addr, 16);
    qemu_print(", PAGE COUNT ");
    qemu_printn(desc->page_count, 10);
    qemu_print("\n");
    page_num += desc->page_count;
  }
  qemu_printn(page_num, 10);
  qemu_print("\n");
  for (int i = 0; i < 32; i++)
  {
    set_idt_entry(i, pf_handler, 0);
  }
  load_idt(idt);
  // setup_paging(bootinfo->mmap, bootinfo->mmap_size);
  // qemu_print("Identity map Initfs ptr");
  // ident_map(bootinfo->ptr_initfs, 3);
  // if (get_map(bootinfo->ptr_initfs) == (uint64_t)bootinfo->ptr_initfs) {
  //   qemu_print("Failed to set memory map");
  //   return 1;
  // }
  // qemu_print("Identity map Initfs ptr + 4096");
  // ident_map(bootinfo->ptr_initfs + 4096, 3);
  // // asm volatile("hlt");

  // qemu_print("Identity map framebuffer base");
  // for (int i = 0; i < bootinfo->size; i += 4096) {
  //   ident_map((void*)(bootinfo->base + (4096 * i)),
  //             3);
  // }
  qemu_print("Map Finished\n");
  // for (;;)
  //   ;
  cpio_header_t* hdr = (cpio_header_t*)bootinfo->ptr_initfs;
  if (hdr->c_magic != 0070707)
  {
    return 1;
  }
  int hdr_size = sizeof(cpio_header_t);
  char* font   = (char*)(bootinfo->ptr_initfs) + hdr_size + hdr->c_namesize + 1;

  init_fb(bootinfo->base,
    bootinfo->pitch,
    bootinfo->horizontal_resolution,
    bootinfo->vertical_resolution);
  init_text(font);
  qemu_print("Starting Kernel\n");
  qemu_print(bootinfo->magic);
  qemu_print("\n");
  qemu_print("Testing Magic Value\n");
  fill(0, 0, fb_xres, fb_yres, 0x000000);
  printf("Framebuffer Base: %x\nFramebuffer Size: %x\n",
    bootinfo->base,
    bootinfo->size);
  qemu_printn(bootinfo->base, 16);

  for (;;);
}
// 11001101000
// 1000 0000 0000 0001 0000 0000 0011 0001
