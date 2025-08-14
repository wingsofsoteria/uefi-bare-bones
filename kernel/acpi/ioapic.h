// clang-format Language: C
#ifndef __KERNEL_ACPI_IOAPIC_H__
#define __KERNEL_ACPI_IOAPIC_H__

#include <stdint.h>
void program_ioapic(uint32_t p_addr, uint32_t gsi_base);
#endif
