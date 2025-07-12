#include <idt.h>
#include <stdint.h>
#include <qemu.h>
struct idt_entry_t
{
  uint16_t address_low;
  uint16_t selector;
  uint8_t ist;
  uint8_t flags;
  uint16_t address_mid;
  uint32_t address_high;
  uint32_t reserved;
} __attribute__((packed));

struct idtr_t
{
  uint16_t limit;
  uint64_t base;
} __attribute__((packed));

struct idt_entry_t idt[256];
struct idtr_t idtr;
void set_idt_entry(uint8_t vector, void* handler, uint8_t dpl)
{
  uint64_t handler_addr = (uint64_t)handler;

  struct idt_entry_t* entry = &idt[vector];
  entry->address_low        = handler_addr & 0xFFFF;
  entry->address_mid        = (handler_addr >> 16) & 0xFFFF;
  entry->address_high       = handler_addr >> 32;
  entry->selector           = 0x8;
  // trap gate + present + DPL
  entry->flags = 0b1110 | ((dpl & 0b11) << 5) | (1 << 7);
  // ist disabled
  entry->ist = 0;
}

void pf_handler()
{
  qemu_printf("Kernel Encountered Page Fault");
  asm volatile("cli; hlt");
}

void load_idt()
{
  for (int i = 0; i < 32; i++)
  {
    set_idt_entry(i, pf_handler, 0);
  }
  idtr.limit = 0xFFF;
  idtr.base  = (uint64_t)(&idt[0]);
  asm volatile("lidt %0" ::"m"(idtr)
    : "memory");
  asm volatile("sti");
}
