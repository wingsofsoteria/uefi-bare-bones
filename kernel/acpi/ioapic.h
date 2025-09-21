// clang-format Language: C
#ifndef __KERNEL_ACPI_IOAPIC_H__
#define __KERNEL_ACPI_IOAPIC_H__

#include <stdint.h>
typedef struct
{
  uint8_t pin;
  uint8_t vector;
  uint8_t delivery_mode : 3;
  uint8_t destination_mode : 1;
  uint8_t delivery_status : 1;
  uint8_t pin_polarity : 1;
  uint8_t remote_irr : 1;
  uint8_t trigger_mode : 1;
  uint8_t mask : 1;
  uint64_t reserved : 39;
  uint8_t destination;
} __attribute__((packed)) ioapic_redtbl_t;
void program_ioapic(uint32_t p_addr, uint32_t gsi_base);
void set_redtbl(uint64_t ioregsel, ioapic_redtbl_t redtbl);
void set_ioapic_addr(uint32_t addr);
void write_ioapic(uint64_t ioregsel, const uint8_t offset, const uint32_t value);
uint32_t read_ioapic(uint64_t ioregsel, const uint8_t offset);
void enable_ps2_keyboard();
#endif
