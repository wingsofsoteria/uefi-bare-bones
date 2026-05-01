#pragma once
#include <stdint.h>

#include "memory/paging.h"

#define PAGE_PRESENT 1

typedef struct {
  uint8_t  present : 1;
  uint8_t  writable : 1;
  uint8_t  user_accessable : 1;
  uint8_t  write_through : 1;
  uint8_t  cache_disable : 1;
  uint8_t  accessed : 1;
  uint8_t  unused_1 : 1;
  uint8_t  page_size : 1;
  uint8_t  unused_2 : 4;
  uint64_t address : 40;
  uint16_t unused_3 : 11;
  uint8_t  execute_disable : 1;
} page_entry_t;

typedef struct {
  page_entry_t pages[PAGE_COUNT];
} page_table_t;

void init_page_table();
