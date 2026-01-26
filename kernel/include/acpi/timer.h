#ifndef __KERNEL_ACPI_TIMER_H__
#define __KERNEL_ACPI_TIMER_H__

#define LAPIC_TIMER_MODE_PERIODIC (1 << 17)
#define LAPIC_TIMER_IRQ           32

void apic_enable_timer();

#endif
