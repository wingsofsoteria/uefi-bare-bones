#pragma once

#include "stdbool.h"

#include <stdint.h>

void* get_mmio_ecam_addr(uint16_t group, uint8_t bus);
bool  init_pci();
void* locate_pci_device(uint32_t class, uint32_t sub_class);
bool  pci_compare_value(
  void*    handle,
  int      register_number,
  uint32_t value,
  uint32_t mask
);

uint32_t get_register_value(void* handle, int register_number);
