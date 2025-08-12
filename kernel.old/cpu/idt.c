#include "graphics.h"
#include "types.h"
#include <cpu.h>
#include <stddef.h>
#include <stdint.h>
#include <qemu.h>
#define IDT_MAX_DESCRIPTORS 32
extern void* isr_stub_table[];
struct idt_entry_t
{
  uint16_t offset_low;
  uint16_t segment_selector;
  uint8_t ist;
  uint8_t flags;
  uint16_t offset_mid;
  uint32_t offset_high;
  uint32_t reserved_high;
} __attribute__((packed));

struct idtr_t
{
  uint16_t limit;
  uint64_t base;
} __attribute__((packed));

__attribute__((aligned(0x10))) static struct idt_entry_t idt[IDT_MAX_DESCRIPTORS];
static struct idtr_t idtr;

void set_idt_entry(uint8_t vector, void* handler, uint8_t gate, uint8_t dpl)
{
  uint64_t handler_addr = (uint64_t)handler;

  struct idt_entry_t entry;
  entry.offset_low       = handler_addr & 0xFFFF;
  entry.segment_selector = 0x08;
  entry.ist              = 0;
  entry.flags            = (1 << 7) | (dpl & 0b11) << 5 | (gate & 0b1111);
  entry.offset_mid       = (handler_addr >> 16) & 0xFFFF;
  entry.offset_high      = (handler_addr >> 32) & 0xFFFFFFFF;
  entry.reserved_high    = 0;
  idt[vector]            = entry;
}

void interrupt_handler(void);
void interrupt_handler(void)
{
  // TODO implement hardware interrupt handling
  return;
}

void exception_handler(void);

void exception_handler()
{
  return;
  // put_pixel(300, 150, 0xFF0000);
}

extern void set_idt(struct idtr_t*);
extern void no_return_loop(void);
void load_idt()
{
  debug("STUB TABLE %x\n", isr_stub_table);
  for (int i = 0; i < IDT_MAX_DESCRIPTORS; i++)
  {
    if (isr_stub_table[i] == NULL)
    {
      debug("ISR %d:%x is null\n", i, isr_stub_table[i]);
      no_return_loop();
    }
    set_idt_entry(i, isr_stub_table[i], 0b1110, 0);
  }
  // for (int i = 32; i < 256; i++)
  // {
  //   set_idt_entry(i, interrupt, 0, 0x8E);
  // }
  idtr.limit = (uint16_t)(sizeof(struct idt_entry_t) * (IDT_MAX_DESCRIPTORS - 1));
  idtr.base  = (uint64_t)(&idt);
  set_idt(&idtr);
}
