#include "gdt.h"
#include "page.h"
#include "qemu.h"
#include <kernel.h>
#include <stdint.h>
#include <stdio.h>
#include <bootinfo.h>
// TODO FIGURE OUT WHY EXCEPTIONS AREN'T BEING HANDLED - FIX IDT CODE SELECTOR
// TODO FIX ISSUE WITH FRAME BUFFER REQUESTING RESTRICTED PAGES
// DONE: IDENTITY MAP PAGES
// UEFI ENABLES PAGING SO SKIP THAT
// PASSED POINTERS TO BOTH ACPI TABLES (ONLY ONE WILL BE VALID, THE OTHER WILL BE NULL BASED ON REVISION)
// TODO TRANSLATE THE EFI MEMORY DESCRIPTORS TO SOMETHING THAT WE CAN USE
// TODO ENABLE ACPI IN THE LOADER

int _start(kernel_bootinfo_t* bootinfo)
{
  qemu_printf("Kernel Start %d\n", -1);
  load_gdt();
  load_idt();
  if (bootinfo->mmap_size == 0)
    return 1;
  loader_memory_descriptor_t* mmap = bootinfo->mmap;

  qemu_printf("Memory Map: %x %d %d\n", (uint64_t)mmap, bootinfo->mmap_size, bootinfo->desc_size);
  uint64_t page_num = 0;
  for (int i = 0; i < (bootinfo->mmap_size / bootinfo->desc_size); i++)
  {
    loader_memory_descriptor_t* desc = (loader_memory_descriptor_t*)((uint8_t*)mmap + (i * bootinfo->desc_size));
    uint64_t paddr                   = desc->p_addr;
    uint64_t vaddr                   = desc->v_addr;
    qemu_printf("Entry %d: Type %d, PADDR %x, VADDR %x, PAGE COUNT %d\n", i, desc->type, desc->p_addr, desc->v_addr, desc->page_count);
    page_num += desc->page_count;
  }
  qemu_printf("Number of pages in memory map: %d\n", page_num);
  qemu_printf("Map Finished\n");
  cpio_header_t* hdr = (cpio_header_t*)bootinfo->initfs;
  if (hdr->c_magic != 0070707)
  {
    return 1;
  }
  int hdr_size = sizeof(cpio_header_t);
  char* font   = (char*)(bootinfo->initfs) + hdr_size + hdr->c_namesize + 1;

  init_fb(bootinfo->base, bootinfo->pitch, bootinfo->horizontal_resolution, bootinfo->vertical_resolution);
  init_text(font);
  qemu_printf("Starting Kernel\n");
  qemu_printf("Got magic value: %s\n", bootinfo->magic);
  fill(0, 0, fb_xres, fb_yres, 0x000000);
  printf("Framebuffer Base: %x\nFramebuffer Size: %x\n", bootinfo->base, bootinfo->size);
  qemu_printf("Framebuffer Base: %x\nFramebuffer Size: %x\n", bootinfo->base, bootinfo->size);
  setup_page_table();
  for (;;);
}
