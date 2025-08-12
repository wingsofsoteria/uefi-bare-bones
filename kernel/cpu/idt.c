#include "cpu/idt.h"
#include <stdbool.h>
#include <stdio.h>

__attribute__((aligned(4096))) static idt_t idt;

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
  printf("Exception");
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
