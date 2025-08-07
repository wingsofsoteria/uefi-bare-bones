#include <kernel.h>
#include <string.h>
#include <stdlib.h>
int kernel_initialization;
void finish_init()
{
  kernel_initialization = 0;
}
int _start(kernel_bootinfo_t* bootinfo)
{
  kernel_initialization = -255;
  if (strncmp(bootinfo->magic, "OS649", 5) != 0)
  {
    qemu_printf("got incorrect magic value %s, aborting...", bootinfo->magic);
    return 1;
  }

  load_gdt();
  load_idt();

  init_fb(bootinfo->base, bootinfo->pitch, bootinfo->horizontal_resolution, bootinfo->vertical_resolution);

  init_cpio(bootinfo->initfs);

  uint8_t* font = get_file();
  init_text(font);

  test_pixels();
  debug("Kernel Start %d\n", -1);
  debug_empty("Starting Kernel\n");
  debug("Got magic value: %s\n", bootinfo->magic);
  // clear_screen();
  debug("Framebuffer Base: %x\nFramebuffer Size: %x\n", bootinfo->base, bootinfo->size);
  setup_page_table(bootinfo->mmap);
  setup_allocator(bootinfo->mmap);
  int* test = malloc(257);
  *test     = 129;
  debug("Malloc test pointer: %x = %d\n", test, *test);
  free(test);
  int* test2 = malloc(257);
  *test2     = 511;
  debug("Malloc test pointer: %x = %d\n", test2, *test2);
  free(test);

  if (bootinfo->xsdt_address != 0)
  {
    xsdt(bootinfo->xsdt_address);
  }

  for (;;);
}
