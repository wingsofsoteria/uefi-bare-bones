#include <qemu.h>
#include <page.h>
#include <stdint.h>
uint64_t p4_table[512];
uint64_t p3_table[512];

void setup_page_table()
{
  uint64_t cr4 = 0;
  asm volatile("mov %%cr4, %0;"
    : "=r"(cr4)
    :
    :);

  qemu_printf("%b\n", cr4);
}
