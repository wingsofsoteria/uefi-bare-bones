#pragma once

#include <stdint.h>

int acpi_init(uint64_t);
void ioapic_enable_irq(int irq, int vector);
void ioapic_disable_irq(int irq);
void lapic_send_eoi();
void lapic_enable();
void lapic_disable();
int uacpi_acpi_init();
void kernel_rsdp_from_bootinfo(void*);
