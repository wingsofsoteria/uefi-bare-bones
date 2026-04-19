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

static void dump_stack(isr_stack_t* stack)
{
  printf("STACK:%x\n", stack);
  printf("\tRFLAGS:%x\n", stack->rflags);
  printf("\t    CS:%x\n", stack->cs);
  printf("\t   RIP:[%x] %s\n", stack->rip, resolve_function_name(stack->rip));
  printf("\t   ERR:%x\n", stack->err);
  printf("\t   ISR:%x\n", stack->isr);
  printf("\t   RAX:%x\n", stack->rax);
  printf("\t   RBX:%x\n", stack->rbx);
  printf("\t   RCX:%x\n", stack->rcx);
  printf("\t   RDX:%x\n", stack->rdx);
  printf("\t   RSI:%x\n", stack->rsi);
  printf("\t   RDI:%x\n", stack->rdi);
  printf("\t   RBP:%x\n", stack->rbp);
  printf("\t    R8:%x\n", stack->r8);
  printf("\t    R9:%x\n", stack->r9);
  printf("\t   R10:%x\n", stack->r10);
  printf("\t   R11:%x\n", stack->r11);
  printf("\t   R12:%x\n", stack->r12);
  printf("\t   R13:%x\n", stack->r13);
  printf("\t   R14:%x\n", stack->r14);
  printf("\t   R15:%x\n", stack->r15);
  uint64_t cr2;
  asm volatile("mov %%cr2, %0"
    : "=r"(cr2));
  printf("\t   CR2:%x\n", cr2);
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
        printf("===GENERAL PROTECTION FAULT===\n\tError Code: %d", stack->err);
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
        printf("Interrupt: %d\n", stack->isr);
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
