#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
struct hash_map;
typedef struct hash_map hash_map_t;

#define hash_map_iter(m, x)   \
  __hash_map_iter_next(m, 1); \
  for (; __hash_map_iter_has_next(m); x = __hash_map_iter_next(m, 0))

bool  __hash_map_iter_has_next(hash_map_t* map);
void* __hash_map_iter_next(hash_map_t* map, int);
void* hash_map_get(hash_map_t* map, char* key, int* out_index);
void  hash_map_resize(hash_map_t* map, int max_cap);
int   hash_map_push(hash_map_t* map, char* key, void* data, size_t data_size);
void* hash_map_pop(hash_map_t* map, char* key);
void* hash_map_create(int max_cap);
void  hash_map_debug(hash_map_t*);
