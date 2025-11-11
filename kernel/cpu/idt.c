#include "cpu/idt.h"
#include "stdlib.h"
#include <stdbool.h>
#include <stdio.h>
#include "../acpi/lapic.h"
#include "keyboard.h"
#include "cpu/task.h"
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

isr_stack_t* interrupt_handler(isr_stack_t* stack)
{
  switch (stack->isr)
  {
    case 32:
      {
        switch_task(stack);
        break;
      }
    case 33:
      {
        uint8_t byte = inb(0x60);
        kb_handle_key(byte);
        break;
      }
    default:
      {
        printf("%d ", stack->isr);
        abort();
      }
  }
  send_eoi();
  return stack;
}

isr_stack_t* exception_handler(isr_stack_t* stack)
{
  if (stack->isr >= 32)
  {
    return interrupt_handler(stack);
  }
  switch (stack->isr)
  {
    case 2:
      {
        printf("GOT NMI\n");
        break;
      }
    case 6:
      {
        break;
      }
    case 13:
      {
        printf("===GENERAL PROTECTION FAULT===\n\tError Code: %d", stack->err);
        halt_cpu
      }
    case 14:
      {
        printf("PAGE FAULT:\n\tErr: %b", stack->err);
        halt_cpu
      }

    default:
      {
        printf("Interrupt: %d\n", stack->isr);
        halt_cpu
      }
  }

  return stack;

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
