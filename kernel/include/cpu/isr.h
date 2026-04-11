#pragma once
#include "cpu/idt.h"

extern volatile interrupt isr_handler_table[256];

isr_stack_t* apic_timer_isr(isr_stack_t*);
isr_stack_t* pic_timer_isr(isr_stack_t*);
isr_stack_t* keyboard_isr(isr_stack_t*);
isr_stack_t* task_switch_isr(isr_stack_t*);
void register_handler(int, interrupt);
void unregister_handler(int);
