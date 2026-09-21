#pragma once

#include "stdbool.h"

#include <stdint.h>

union pci_register
{
  struct
  {
    uint16_t lower;
    uint16_t upper;
  } __attribute__((packed)) bits_16;

  struct
  {
    uint8_t lower;
    uint8_t lower_mid;
    uint8_t upper_mid;
    uint8_t upper;
  } __attribute((packed)) bits_8;

  uint32_t bits_32;
};

struct pci_device_descriptor
{
  uint64_t           ecam_base_addr;
  uint8_t            bus;
  uint8_t            device;
  uint8_t            function;
  union pci_register registers[0x12];
};

struct pci_device_pointer
{
  uint64_t ecam_base_addr;
  uint8_t  bus;
  uint8_t  device;
  uint8_t  function;
};

uint64_t get_mmio_ecam_addr(uint8_t bus);
bool     init_pci();
