#include "hashmap.h"

#include "fnv.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
  char key[1022];
  void*       data;
} hash_entry;

typedef struct hash_map
{
  int         capacity;
  int         count;
  hash_entry* inner;
} hash_map_t;

void hash_map_debug(hash_map_t* map)
{
  printf("Map(%i) %i\n", map->capacity, map->count);
  for (int i = 0; i < map->capacity; i++)
    {
      hash_entry entry = map->inner[i];
      if (!entry.data)
        {
          printf("Empty(%i)\n", i);
          continue;
        }
      uint32_t hash = fnv_32a_str(entry.key);
      printf(
        "%s(%x:%d:%d) -> %p\n",
        entry.key,
        hash,
        hash % map->capacity,
        i,
        entry.data
      );
    }
}

void* hash_map_get(hash_map_t* map, char* key, int* out_index)
{
  uint32_t hash  = fnv_32a_str(key);
  int      index = hash % map->capacity;
  while (index < map->capacity)
    {
      if (!map->inner[index].data)
        {
          index++;
          continue;
        }
      hash_entry entry      = map->inner[index];
      uint32_t   entry_hash = fnv_32a_str(entry.key);
      if (entry_hash == hash)
        {
          *out_index = index;
          return entry.data;
        }
      index++;
    }

  return NULL;
}

void hash_map_resize(hash_map_t* map, int max_cap)
{
  assert(max_cap > map->capacity);
  hash_entry* inner = calloc(max_cap, sizeof(hash_entry));
  for (int i = 0; i < map->capacity; i++)
    {
      if (!map->inner[i].data) { continue; }
      uint32_t hash  = fnv_32a_str(map->inner[i].key);
      int      index = hash % max_cap;
      while (inner[index].data)
        {
          index++;
          if (index >= max_cap)
            {
              free(inner);
              hash_map_resize(map, max_cap * 2);
              return;
            }
        }
      assert(!inner[index].data);
      inner[index].data = map->inner[i].data;
    memcpy(inner[index].key, map->inner[i].key, 1022);
    }

  free(map->inner);
  map->inner    = inner;
  map->capacity = max_cap;
}

int hash_map_push(hash_map_t* map, char* key, void* data, size_t data_size)
{
  if (map->count >= map->capacity) { hash_map_resize(map, map->capacity * 2); }
  uint32_t hash   = fnv_32a_str(key);
  int      ignore = 0;
  if (hash_map_get(map, key, &ignore) != NULL) { return 1; }
  int index = hash % map->capacity;
  while (map->inner[index].data != NULL)
    {
      index++;
      if (index >= map->capacity) { hash_map_resize(map, map->capacity * 2); }
    }
  assert(!map->inner[index].data);
  map->inner[index].data = data;
  memcpy(map->inner[index].key, key, strlen(key));
  map->count++;
  return 0;
}

void* hash_map_pop(hash_map_t* map, char* key)
{
  int   index      = -1;
  void* entry_data = hash_map_get(map, key, &index);
  if (entry_data == NULL) { return NULL; }
  if (index == -1) { return NULL; }
  map->inner[index].data = NULL;
  memset(map->inner[index].key, 0, 1022);
  map->count--;
  return entry_data;
}

int hash_map_create(hash_map_t** out_map, int max_cap)
{
  hash_map_t* map = malloc(sizeof(hash_map_t));
  if (map == NULL) { return 1; }
  map->capacity = max_cap;
  map->count    = 0;
  map->inner    = calloc(max_cap, sizeof(hash_entry));
  memset(map->inner, 0, max_cap * sizeof(hash_entry));
  if (map->inner == NULL)
    {
      free(map);
      return 1;
    }
  *out_map = map;
  return 0;
}
