#pragma once

#include <stdint.h>

struct mmio_configuration_space
{
  uint64_t base_address;
  uint16_t group_num;
  uint8_t  bus_start;
  uint8_t  bus_end;
  uint32_t reserved;
} __attribute__((packed));
