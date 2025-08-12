#include "types.h"
#include <cpu.h>
#include <stdint.h>

#define PRESENT_BIT    0b10000000
#define USER_PRIVILEGE 0b01100000
#define NORMAL_SEGMENT 0b00010000
#define CODE_SEGMENT   0b00001000
#define GROWS_DOWN     0b00000100
#define READ_WRITE     0b00000010
#define ACCESSED       0b00000001

#define PAGE_GRANULARITY 0b1000
#define SEGMENT_32_BIT   0b0100
#define LONG_MODE        0b0010

struct gdt_entry
{
  uint16_t limit_low;
  uint16_t base_low;
  uint8_t base_mid;
  uint8_t access;
  uint8_t flags_and_limit_high;
  uint8_t base_high;
} __attribute__((packed));
struct gdt_ptr
{
  uint16_t limit;
  uint64_t base;
} __attribute__((packed));

static struct gdt_entry gdt[3];
static struct gdt_ptr gp;
extern void reload_segments(struct gdt_ptr*);

void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags)
{
  struct gdt_entry entry;
  entry.limit_low            = limit & 0xFFFF;
  entry.base_low             = base & 0xFFFF;
  entry.base_mid             = (base >> 16) & 0xFF;
  entry.access               = access;
  entry.flags_and_limit_high = flags << 4 | (limit >> 16) & 0xF;
  entry.base_high            = (base >> 24) & 0xFF;

  gdt[num] = entry;
}

void gdt_set_gate_simple(int num, uint8_t access, uint8_t flags)
{
  struct gdt_entry entry;
  entry.limit_low            = 0xFFFF;
  entry.base_low             = 0;
  entry.base_mid             = 0;
  entry.access               = access;
  entry.flags_and_limit_high = flags << 4 | 0xF;
  entry.base_high            = 0;

  gdt[num] = entry;
}

void load_gdt()
{
  gp.limit = (sizeof(uint64_t) * 3) - 1;
  gp.base  = ((uint64_t)&gdt);

  gdt_set_gate_simple(0, 0, 0);
  gdt_set_gate_simple(1, PRESENT_BIT | NORMAL_SEGMENT | CODE_SEGMENT | READ_WRITE, PAGE_GRANULARITY | LONG_MODE);
  gdt_set_gate_simple(2, PRESENT_BIT | NORMAL_SEGMENT | READ_WRITE, PAGE_GRANULARITY | SEGMENT_32_BIT);
  // gdt_set_gate(3, 0, 0, PRESENT_BIT | USER_PRIVILEGE | NORMAL_SEGMENT | CODE_SEGMENT | READ_WRITE | ACCESSED, PAGE_GRANULARITY | LONG_MODE);
  // gdt_set_gate(4, 0, 0, PRESENT_BIT | USER_PRIVILEGE | NORMAL_SEGMENT | READ_WRITE | ACCESSED, PAGE_GRANULARITY | SEGMENT_32_BIT);
  reload_segments(&gp);
}
