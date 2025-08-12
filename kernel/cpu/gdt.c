#include "gdt.h"
#include <kernel.h>
#include <stdint.h>

typedef struct
{
  uint16_t limit_low;
  uint16_t base_low;
  uint8_t base_mid;
  uint8_t access;
  uint8_t flags_and_limit_high;
  uint8_t base_high;
} __attribute__((packed)) gdt_entry_t;

typedef struct
{
  gdt_entry_t null;
  gdt_entry_t kernel_code;
  gdt_entry_t kernel_data;
  gdt_entry_t user_data;
  gdt_entry_t user_code;
} __attribute__((packed)) gdt_t;

typedef struct
{
  uint16_t size;
  uint64_t offset;
} __attribute__((packed)) gdt_ptr_t;

extern void set_gdt(gdt_ptr_t*);

__attribute__((aligned(4096))) static gdt_t gdt;

static gdt_entry_t gdt_set_gate_simple(uint8_t access, uint8_t flags)
{
  gdt_entry_t entry;
  entry.limit_low            = 0;
  entry.base_low             = 0;
  entry.base_mid             = 0;
  entry.access               = access;
  entry.flags_and_limit_high = flags << 4;
  entry.base_high            = 0;

  return entry;
}

void load_gdt()
{
  gdt.null        = gdt_set_gate_simple(0, 0);      // 0x0
  gdt.kernel_code = gdt_set_gate_simple(0x9A, 0xA); // 0x08
  gdt.kernel_data = gdt_set_gate_simple(0x92, 0xC); // 0x10
  gdt.user_data   = gdt_set_gate_simple(0xF2, 0xC); // 0x18
  gdt.user_code   = gdt_set_gate_simple(0xFA, 0xA); // 0x20

  gdt_ptr_t ptr;
  ptr.size   = sizeof(gdt_t) - 1;
  ptr.offset = (uint64_t)&gdt;
  set_gdt(&ptr);
}
