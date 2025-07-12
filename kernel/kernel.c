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
// TODO FORMAT QEMU_PRINT* FUNCTIONS LIKE PRINTF SO KERNEL ISN'T POLLUTED

int _start(kernel_bootinfo_t* bootinfo)
{
  qemu_print("Kernel Start\n");
  if (bootinfo->mmap_size == 0)
    return 1;
  loader_memory_descriptor_t* mmap = bootinfo->mmap;

  qemu_printn((uint64_t)mmap, 16);
  qemu_print(" ");
  qemu_printn(bootinfo->mmap_size, 10);
  qemu_print(" ");
  qemu_printn(bootinfo->desc_size, 10);
  qemu_print("\n");
  uint64_t page_num = 0;
  for (int i = 0; i < (bootinfo->mmap_size / bootinfo->desc_size); i++)
  {
    loader_memory_descriptor_t* desc = (loader_memory_descriptor_t*)((uint8_t*)mmap + (i * bootinfo->desc_size));
    uint64_t paddr                   = desc->p_addr;
    uint64_t vaddr                   = desc->v_addr;
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

  qemu_print("Map Finished\n");
  cpio_header_t* hdr = (cpio_header_t*)bootinfo->initfs;
  if (hdr->c_magic != 0070707)
  {
    return 1;
  }
  int hdr_size = sizeof(cpio_header_t);
  char* font   = (char*)(bootinfo->initfs) + hdr_size + hdr->c_namesize + 1;

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
