#pragma once

#include <stdint.h>

int acpi_late_init();
int acpi_early_init(void*);
void ioapic_enable_irq(int irq, int vector);
void ioapic_disable_irq(int irq);
void lapic_send_eoi();
void lapic_enable();
void lapic_disable();
void shutdown();
