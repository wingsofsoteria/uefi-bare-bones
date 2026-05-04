#pragma once
#include <stddef.h>
struct hash_map;
typedef struct hash_map hash_map_t;

void* hash_map_get(hash_map_t* map, char* key, int* out_index);
void  hash_map_resize(hash_map_t* map, int max_cap);
int   hash_map_push(hash_map_t* map, char* key, void* data, size_t data_size);
void* hash_map_pop(hash_map_t* map, char* key);
int   hash_map_create(hash_map_t** out_map, int max_cap);
void  hash_map_debug(hash_map_t*);
