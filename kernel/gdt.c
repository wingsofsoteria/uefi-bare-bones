#include <cpu.h>
#include <stdint.h>

struct gdt_entry
{
  uint16_t limit_low;
  uint16_t base_low;
  uint8_t base_middle;
  uint8_t access;
  uint8_t granularity;
  uint8_t base_high;
} __attribute__((packed));

struct gdt_ptr
{
  uint16_t limit;
  uint64_t base;
} __attribute__((packed));

struct gdt_entry gdt[5];
struct gdt_ptr gp;

extern void reload_segments(struct gdt_ptr*);

void gdt_set_gate(int num, uint8_t access, uint8_t granularity)
{
  gdt[num].base_low    = 0;
  gdt[num].base_middle = 0;
  gdt[num].base_high   = 0;

  gdt[num].limit_low   = 0;
  gdt[num].granularity = (granularity << 4);

  gdt[num].access = access;
}

// 7 6 5 4 3  2  1 0
// P DPL S E DC RW A

// 3 2  1 0
// G DB L 0

void load_gdt()
{
  gp.limit = (sizeof(struct gdt_entry) * 5) - 1;
  gp.base  = (uint64_t)&gdt;

  gdt_set_gate(0, 0, 0);
  gdt_set_gate(1, 0x9A, 0xA); // present, ring 0, not system segment, code, conform, R/W | page granularity, long mode
  gdt_set_gate(2, 0x92, 0xC); // present, ring 0, not system segment, data, grows up, R/W | page granularity, data segment
  gdt_set_gate(3, 0xFA, 0xA); // present, ring 3, not system segment, code, conform, R/W | page granularity, long mode
  gdt_set_gate(4, 0xF2, 0xC); // present, ring 3, not system segment, data, grows up, R/W | page granularity, data segment
  reload_segments(&gp);
}
