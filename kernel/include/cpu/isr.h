#pragma once
#include "cpu/idt.h"

extern volatile interrupt isr_handler_table[256];

void register_handler(int, interrupt);
void unregister_handler(int);
