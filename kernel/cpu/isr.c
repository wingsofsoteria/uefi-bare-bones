#include "cpu/isr.h"
#include "config.h"
#include "cpu/idt.h"
#include "cpu/task.h"

#include <keyboard.h>
#include <stdlib.h>

volatile interrupt isr_handler_table[256];

void unregister_handler(int vector)
{
  isr_handler_table[vector] = NULL;
}

void register_handler(int vector, interrupt handler)
{
  isr_handler_table[vector] = handler;
}

isr_stack_t* apic_timer_isr(isr_stack_t* stack)
{
  if (ticks > 0) ticks--;
  if (kernel_config & INTERRUPT_CONFIG_ENABLE_TASKING)
  {
    switch_task(stack);
  }
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
