// NOLINTBEGIN(misc-use-internal-linkage)
#include "cpu/idt.h"
#include "cpu/isr.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <config.h>
#include <acpi/acpi.h>
#include <stdio.h>
#include <keyboard.h>
#include "trace.h"
volatile uint64_t ticks;
volatile uint8_t tsc_waiting;
__attribute__((aligned(4096))) static idt_t idt;

static isr_stack_t* blank_handler(isr_stack_t* stack)
{
  return stack;
}

void disable_irq(int irq, int vector)
{
  if (kernel_config.interrupt_source == 0b10)
  {
    ioapic_disable_irq(irq);
  }

  unregister_handler(vector);
}
// TODO add support for PIC
void enable_irq(int irq, int vector, interrupt handler)
{
  if (kernel_config.interrupt_source == 0b10)
  {
    ioapic_enable_irq(irq, vector);
  }

  register_handler(vector, handler);
}
#define stack_member(x) #x, stack->x
static void dump_stack(isr_stack_t* stack)
{
  printf("STACK:%p\n", stack);
  printf("%9s:%lx\n", stack_member(rflags));
  printf("%9s:%lx\n", stack_member(cs));
  printf(
    "%9s:[%lx] %s\n", stack_member(rip), resolve_function_name(stack->rip));
  printf("%9s:%ld\n", stack_member(err));
  printf("%9s:%lx\n", stack_member(isr));
  printf("%9s:%lx\n", stack_member(rax));
  printf("%9s:%lx\n", stack_member(rbx));
  printf("%9s:%lx\n", stack_member(rcx));
  printf("%9s:%lx\n", stack_member(rdx));
  printf("%9s:%lx\n", stack_member(rsi));
  printf("%9s:%lx\n", stack_member(rdi));
  printf("%9s:%lx\n", stack_member(rbp));
  printf("%9s:%lx\n", stack_member(r8));
  printf("%9s:%lx\n", stack_member(r9));
  printf("%9s:%lx\n", stack_member(r10));
  printf("%9s:%lx\n", stack_member(r11));
  printf("%9s:%lx\n", stack_member(r12));
  printf("%9s:%lx\n", stack_member(r13));
  printf("%9s:%lx\n", stack_member(r14));
  printf("%9s:%lx\n", stack_member(r15));
  uint64_t cr2;
  asm volatile("mov %%cr2, %0"
    : "=r"(cr2));
  printf("%9s:%lx\n", "cr2", cr2);
  walk_stack();
}

static void set_idt_entry_simple(uint8_t vector, void* handler)
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

static isr_stack_t* interrupt_handler(isr_stack_t* stack)
{
  isr_stack_t* modified_stack = isr_handler_table[stack->isr](stack);
  lapic_send_eoi();
  return modified_stack;
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
    case 8:
      {
        printf("===DOUBLE FAULT===\n");
        halt_cpu
      }
    case 13:
      {
        printf("===GENERAL PROTECTION FAULT===\n\tError Code: %lu", stack->err);
        halt_cpu
      }
    case 14:
      {
        printf("PAGE FAULT\n");
        dump_stack(stack);
        halt_cpu
      }

    default:
      {
        printf("Interrupt: %lu\n", stack->isr);
        halt_cpu
      }
  }

  return stack;
}

void load_idt()
{
  for (int i = 0; i < IDT_ENTRY_COUNT; i++)
  {
    set_idt_entry_simple(i, isr_stub_table[i]);
  }
  idt_ptr_t ptr;
  ptr.size   = (sizeof(idt_t)) - 1;
  ptr.offset = (uint64_t)&idt;
  set_idt(&ptr);
}
// NOLINTEND(misc-use-internal-linkage)
