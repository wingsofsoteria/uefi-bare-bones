#ifndef __KERNEL_CPU_ISR_H__
#define __KERNEL_CPU_ISR_H__

#include "cpu/idt.h"

extern volatile interrupt isr_handler_table[256];

isr_stack_t* apic_timer_isr(isr_stack_t*);
isr_stack_t* pic_timer_isr(isr_stack_t*);
isr_stack_t* keyboard_isr(isr_stack_t*);
#endif
