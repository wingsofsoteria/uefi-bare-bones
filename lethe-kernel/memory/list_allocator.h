#pragma once
#include <stdint.h>

typedef struct list_node
{
  int pages;
  struct list_node* next;
} list_node_t;

typedef struct
{
  list_node_t head;
} list_allocator_t;

void add_region(uint64_t, int);
void* list_alloc(int);
