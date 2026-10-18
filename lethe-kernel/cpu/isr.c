#include "cpu/isr.h"

#include "cpu/idt.h"
#include "stddef.h"

#include <keyboard.h>

volatile interrupt isr_handler_table[IDT_ENTRY_COUNT];

void unregister_handler(int vector) { isr_handler_table[vector] = NULL; }

void register_handler(int vector, interrupt handler)
{ isr_handler_table[vector] = handler; }

