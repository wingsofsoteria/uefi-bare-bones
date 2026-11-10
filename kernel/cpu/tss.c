#include "cpu/tss.h"

#include "cpu/gdt.h"

#include <stdint.h>
#include <string.h>
static tss_t tss;
extern void  flush_tss();

void init_tss()
{
  memset(&tss, 0, sizeof(tss_t));
  tss.iomap_base = sizeof(tss_t);
  gdt_set_tss(&tss);
  flush_tss();
}

void set_kernel_tss(uint64_t stack) { tss.rsp0 = stack; }

