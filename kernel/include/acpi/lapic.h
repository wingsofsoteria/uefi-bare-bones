// clang-format Language: C
#ifndef __KERNEL_ACPI_LAPIC_H__
#define __KERNEL_ACPI_LAPIC_H__

#include <stdint.h>
#define LAPIC_TIMER_INITIAL_COUNT_REGISTER 0x380
#define LAPIC_TIMER_CURRENT_COUNT_REGISTER 0x390
#define LAPIC_TIMER_DIVIDE_CONFIG_REGISTER 0x3E0
#define LAPIC_TIMER_REGISTER               0x320
#define LAPIC_INTERRUPT_MASK               0x10000
void lapic_write(uint16_t offset, uint32_t value);
uint32_t lapic_read(uint16_t offset);
void lapic_enable();
void lapic_send_eoi();
void lapic_set_addr(uint32_t addr);
void lapic_disable();

#endif
