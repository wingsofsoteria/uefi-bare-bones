#include <gdt.h>
struct gdt_entry
{
  unsigned short limit_low;
  unsigned short base_low;
  unsigned char base_middle;
  unsigned char access;
  unsigned char granularity;
  unsigned char base_high;
} __attribute__((packed));

struct gdt_ptr
{
  unsigned short limit;
  unsigned int base;
} __attribute__((packed));

struct gdt_entry gdt[5];
struct gdt_ptr gp;

void
gdt_set_gate(int num,
  unsigned long base,
  unsigned long limit,
  unsigned char access,
  unsigned char granularity)
{
  gdt[num].base_low    = base & 0xFFFF;
  gdt[num].base_middle = (base >> 16) & 0xFF;
  gdt[num].base_high   = (base >> 24) & 0xFF;

  gdt[num].limit_low   = limit & 0xFFFF;
  gdt[num].granularity = (limit >> 16) & 0xFF;

  gdt[num].granularity |= (granularity & 0xF0);
  gdt[num].access       = access;
}

// 7 6 5 4 3  2  1 0
// P DPL S E DC RW A

// 3 2  1 0
// G DB L 0

void
load_gdt()
{
  gp.limit = (sizeof(struct gdt_entry) * 3) - 1;
  gp.base  = (unsigned int)&gdt;

  gdt_set_gate(0, 0, 0, 0, 0);
  gdt_set_gate(1, 0, 0xFFFFFFFF, 0b10011010, 0b00001010); // present, ring 0, not system segment, code, conform, R/W | page granularity, long mode
  gdt_set_gate(2, 0, 0xFFFFFFFF, 0b10010010, 0b00001100); // present, ring 0, not system segment, data, grows up, R/W | page granularity, data segment
  gdt_set_gate(3, 0, 0xFFFFFFFF, 0b11111010, 0b00001010); // present, ring 3, not system segment, code, conform, R/W | page granularity, long mode
  gdt_set_gate(4, 0, 0xFFFFFFFF, 0b11110010, 0b00001100); // present, ring 3, not system segment, data, grows up, R/W | page granularity, data segment
  asm volatile("lgdt %0;"
    :
    : "m"(gp)
    : "memory");
}
