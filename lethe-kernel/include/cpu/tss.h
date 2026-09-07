#pragma once

#include <stdint.h>

typedef struct
{
  uint32_t reserved_1;
  uint64_t rsp0;
  uint64_t rsp1;
  uint64_t rsp2;
  uint64_t reserved_2;
  uint64_t ist1;
  uint64_t ist2;
  uint64_t ist3;
  uint64_t ist4;
  uint64_t ist5;
  uint64_t ist6;
  uint64_t ist7;
  uint64_t reserved_3;
  uint16_t reserved_4;
  uint16_t iomap_base;
} __attribute__((__packed__)) tss_t;

void set_kernel_tss(uint64_t);
void init_tss();
