#ifndef __KERNEL_ACPI_H__
#define __KERNEL_ACPI_H__

#include "types.h"
#include <stdint.h>

int acpi_init(void*);
void ioapic_enable_irq(int irq, int vector);
void ioapic_disable_irq(int irq);
void lapic_send_eoi();
void lapic_enable();
void lapic_disable();
void kernel_rsdp_from_bootinfo(kernel_bootinfo_t*);
#endif
