#include "cpu/idt.h"
#include "stdlib.h"
#include <stdbool.h>
#include <stdio.h>
#include "../acpi/lapic.h"
#include "keyboard.h"
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

void interrupt_handler(uint64_t isr)
{
  switch (isr)
  {
    case 33:
      {
        uint8_t byte = inb(0x60);
        char ch      = scancode_to_char(byte);
        if (ch != 0)
        {
          printf("%c", ch);
        }
        break;
      }
    default:
      {
        printf("%d ", isr);
      }
  }

  send_eoi();
}

void exception_handler(isr_stack_t* stack)
{
  if (stack->isr >= 32)
  {
    interrupt_handler(stack->isr);
    return;
  }
  printf("Interrupt: %d\n", stack->isr);
  switch (stack->isr)
  {
    case 2:
      {
        printf("GOT NMI\n");
        return;
      }
    case 14:
      {
        printf("PAGE FAULT:\n\tErr: %b", stack->err);
        halt_cpu
      }

    default:
      {
        halt_cpu
      }
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
