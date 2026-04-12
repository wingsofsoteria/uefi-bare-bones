#pragma once
#include <stdint.h>
typedef struct __list_node
{
  uint64_t size;
  struct __list_node* next;
} list_node_t;

typedef struct
{
  list_node_t head;
} list_allocator_t;

void add_region(uint64_t, uint64_t);
void* list_alloc(uint64_t);
