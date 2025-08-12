#include <string.h>
#include <types.h>
#include <debug.h>
#include <stddef.h>
#include <stdint.h>
#include "idt.h"
#include <kernel.h>
#include "stdlib.h"

extern void* isr_stub_table[256];

typedef struct
{
  uint16_t size;
  uint64_t offset;
} __attribute__((packed)) idt_ptr_t;

typedef struct
{
  uint16_t offset_low;
  uint16_t segment_selector;
  uint8_t ist;
  uint8_t flags;
  uint16_t offset_mid;
  uint32_t offset_high;
  uint32_t reserved_high;
} __attribute__((packed)) idt_entry_t;

typedef struct
{
  idt_entry_t entries[256];
} __attribute__((packed)) idt_t;

__attribute__((aligned(4096))) static idt_t idt;

extern void set_idt(idt_ptr_t*);

void set_idt_entry_simple(uint8_t vector, void* handler)
{
  idt_entry_t* entry      = &(idt.entries[vector]);
  entry->offset_low       = (uint64_t)handler & 0xFFFF;
  entry->segment_selector = 0x08;
  entry->ist              = 0;
  entry->flags            = 0x8E;
  entry->offset_mid       = ((uint64_t)handler >> 16) & 0xFFFF;
  entry->offset_high      = ((uint64_t)handler >> 32) & 0xFFFFFFFF;
  entry->reserved_high    = 0;
}

void exception_handler(void* stack)
{
  debug_empty("Exception");
  while (true)
  {
    asm volatile("hlt");
  }
} // TODO stack data type + proper per exception handling

void load_idt()
{
  for (int i = 0; i < 256; i++)
  {
    set_idt_entry_simple(i, isr_stub_table[i]);
  }
  idt_ptr_t ptr;
  ptr.size   = (sizeof(idt_t)) - 1;
  ptr.offset = (uint64_t)&idt;
  set_idt(&ptr);
}
