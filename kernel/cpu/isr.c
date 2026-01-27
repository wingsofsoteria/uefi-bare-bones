#include "cpu/isr.h"
#include "cpu/idt.h"
#include "cpu/task.h"
#include "keyboard.h"

typedef isr_stack_t* (*interrupt)(isr_stack_t*);

volatile interrupt isr_handler_table[256];

isr_stack_t* apic_timer_isr(isr_stack_t* stack)
{
  // switch_task(stack);
  return stack;
}

isr_stack_t* pic_timer_isr(isr_stack_t* stack)
{
  if (ticks > 0) ticks--;
  return stack;
}

isr_stack_t* keyboard_isr(isr_stack_t* stack)
{
  kb_handle_key();
  return stack;
}
