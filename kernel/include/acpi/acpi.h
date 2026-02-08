#ifndef __KERNEL_ACPI_H__
#define __KERNEL_ACPI_H__

#include <stdint.h>

void acpi_init(uint64_t);
void ioapic_enable_irq(int irq, int vector);
void ioapic_disable_irq(int irq);
void lapic_send_eoi();
void lapic_enable();
void loop_print_definition_blocks();
#endif
