#include "cpu/gdt.h"

#include "cpu/tss.h"

#include <stdint.h>

__attribute__((aligned(4096))) static gdt_t gdt;

static gdt_long_entry_t gdt_set_long_gate(
  uint64_t base,
  uint32_t limit,
  uint8_t  access,
  uint8_t  flags
)
{
  gdt_long_entry_t entry;
  entry.limit_low            = limit & 0xFFFF;
  entry.base_low             = base & 0xFFFF;
  entry.base_low_mid         = (base >> 16) & 0xFF;
  entry.access               = access;
  entry.flags_and_limit_high = ((flags << 4) & 0xF0) | ((limit >> 16) & 0x0F);
  entry.base_upper_mid       = (base >> 24) & 0xFF;
  entry.base_high            = (base >> 32) & 0xFFFFFFFF;
  entry.reserved             = 0;
  return entry;
}

static gdt_entry_t gdt_set_gate(
  uint32_t base,
  uint32_t limit,
  uint8_t  access,
  uint8_t  flags
)
{
  gdt_entry_t entry;
  entry.limit_low            = limit & 0xFFFF;
  entry.flags_and_limit_high = ((limit >> 16) & 0xF) | flags << 4;
  entry.base_low             = base & 0xFFFF;
  entry.base_mid             = (base >> 16) & 0xFF;
  entry.base_high            = (base >> 24) & 0xFF;
  entry.access               = access;
  return entry;
}

static gdt_entry_t gdt_set_gate_simple(uint8_t access, uint8_t flags)
{ return gdt_set_gate(0, 0, access, flags); }

void gdt_set_tss(tss_t* tss)
{ gdt.tss = gdt_set_long_gate((uint64_t)tss, sizeof(tss_t) - 1, 0x89, 0); }

void load_gdt()
{
  gdt.null        = gdt_set_gate_simple(0, 0);      // 0x0
  gdt.kernel_code = gdt_set_gate_simple(0x9A, 0xA); // 0x08
  gdt.kernel_data = gdt_set_gate_simple(0x92, 0xC); // 0x10
  gdt.user_code   = gdt_set_gate_simple(0xFA, 0xA);
  gdt.user_data   = gdt_set_gate_simple(0xF2, 0xC);
  gdt.tss         = gdt_set_long_gate(0, 0, 0x89, 0x0);
  gdt_ptr_t ptr;
  ptr.size   = sizeof(gdt_t) - 1;
  ptr.offset = (uint64_t)&gdt;
  set_gdt(&ptr);
}
