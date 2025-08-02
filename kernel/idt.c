#include <cpu.h>
#include <stddef.h>
#include <stdint.h>
#include <qemu.h>
#include <stdio.h>

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

struct interrupt_frame
{
  size_t ip;
  size_t cs;
  size_t flags;
  size_t sp;
  size_t ss;
};

struct idtr_t
{
  uint16_t limit;
  uint64_t base;
} __attribute__((packed));

struct idt_entry_t idt[256];
struct idtr_t idtr;
extern void set_idt_entries(struct idtr_t*);
void set_idt_entry(uint8_t vector, void* handler, uint8_t dpl, uint8_t flags)
{
  uint64_t handler_addr = (uint64_t)handler;

  struct idt_entry_t* entry = &idt[vector];
  entry->address_low        = handler_addr & 0xFFFF;
  entry->selector           = 0x08;
  entry->ist                = 0;
  entry->flags              = flags;
  entry->address_mid        = (handler_addr >> 16) & 0xFFFF;
  entry->address_high       = (handler_addr >> 32) & 0xFFFFFFFF;
  entry->reserved           = 0;
}

__attribute__((interrupt)) void interrupt(struct interrupt_frame* frame)
{
  return;
}

__attribute__((interrupt)) void exception(struct interrupt_frame* frame, size_t error_code)
{
  for (;;);
}

void load_idt()
{
  for (int i = 0; i < 32; i++)
  {
    set_idt_entry(i, exception, 0, 0x8E);
  }
  idtr.limit = (uint16_t)(sizeof(struct idt_entry_t) * 256) - 1;
  idtr.base  = (uint64_t)(&idt[0]);
  set_idt_entries(&idtr);
}
