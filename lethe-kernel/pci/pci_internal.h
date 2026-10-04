#pragma once

#include <stdint.h>
#define CONFIG_ADDRESS 0xCF8
#define CONFIG_DATA    0xCFC

struct pci_bus
{
  struct pci_device* parent;
  struct pci_device* dev_list;
  struct pci_bus*    next;
  uint8_t            index;
};

struct pci_device
{
  struct pci_device_descriptor* descriptor;
  struct pci_device*            parent_device;
  struct pci_bus*               parent_bus;
  struct pci_device*            next;
  struct pci_device*            functions;
};

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
  uint8_t            bus;
  uint8_t            device;
  uint8_t            function;
  union pci_register registers[0x12];
};

