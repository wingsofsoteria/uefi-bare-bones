// clang-format Language: C
#ifndef __KERNEL_ACPI_LAPIC_H__
#define __KERNEL_ACPI_LAPIC_H__

#include <stdint.h>
void lapic_write(uint16_t offset, uint32_t value);
uint32_t lapic_read(uint16_t offset);
void lapic_enable(uint32_t addr);
void send_eoi();
#endif
