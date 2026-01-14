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

#endif
