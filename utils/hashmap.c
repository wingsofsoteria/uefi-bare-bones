#include "hashmap.h"

#include "fnv.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define RESIZE_VAL       2
#define INITIAL_CAPACITY 4
#define LOAD_FACTOR      0.75F
#define hash_value(x)    fnv_64a_buf(x, KEY_LEN)

extern void* calloc(size_t, size_t);
extern void* malloc(size_t);
extern void  free(void*);

typedef struct
{
  const char* key;
  void*       value;
} hm_entry;

struct hm
{
  hm_entry* entries;
  size_t    capacity;
  size_t    count;
};

static char* hm_strdup(const char* str)

{
  char* new_str = calloc(KEY_LEN, sizeof(char));
  if (!new_str) { return NULL; }
  memcpy(new_str, str, KEY_LEN);
  return new_str;
}

static const char* set_entry(
  hm_entry*   entries,
  size_t      capacity,
  const char* key,
  void*       value,
  size_t*     plength
)
{
  uint64_t hash  = hash_value(key);
  size_t   index = hash % capacity;
  while (entries[index].key != NULL)
    {
      if (memcmp(key, entries[index].key, KEY_LEN) == 0)
        {
          entries[index].value = value;
          return entries[index].key;
        }
      index = (index + 1) % capacity;
    }
  if (plength != NULL)
    {
      key = hm_strdup(key);
      if (!key) return NULL;
      (*plength)++;
    }
  entries[index].key   = (char*)key;
  entries[index].value = value;
  return key;
}

static bool hm_expand(hm* map)
{
  size_t capacity = map->capacity * RESIZE_VAL;
  if (capacity < map->capacity) { return false; }
  hm_entry* entries = calloc(capacity, sizeof(hm_entry));
  if (!entries) { return false; }
  for (size_t i = 0; i < map->capacity; i++)
    {
      hm_entry entry = map->entries[i];
      if (entry.key != NULL)
        {
          set_entry(entries, capacity, entry.key, entry.value, NULL);
        }
    }
  free(map->entries);
  map->capacity = capacity;
  map->entries  = entries;
  return true;
}

hm* hm_create()
{
  hm* map = malloc(sizeof(hm));
  if (!map) { return NULL; }
  map->count    = 0;
  map->capacity = INITIAL_CAPACITY;
  map->entries  = calloc(INITIAL_CAPACITY, sizeof(hm_entry));
  if (!map->entries)
    {
      free(map);
      return NULL;
    }
  return map;
}

void* hm_get(hm* map, const char* key)
{
  uint64_t hash  = hash_value(key);
  size_t   index = hash % map->capacity;
  while (map->entries[index].key != NULL)
    {
      if (memcmp(map->entries[index].key, key, KEY_LEN) == 0)
        {
          return map->entries[index].value;
        }
      index = (index + 1) % map->capacity;
    }

  return NULL;
}

const char* hm_set(hm* map, const char* key, void* value)
{
  assert(value);
  if (map->count >= (map->capacity * LOAD_FACTOR))
    {
      if (!hm_expand(map)) { return NULL; }
    }
  return set_entry(map->entries, map->capacity, key, value, &map->count);
}

hmi hm_iter(hm* map)
{
  hmi it;
  it._map   = map;
  it._index = 0;
  return it;
}

bool hm_next(hmi* it)
{
  hm* map = it->_map;
  while (it->_index < map->capacity)
    {
      size_t i = it->_index;
      it->_index++;
      if (map->entries[i].key)
        {
          hm_entry entry = map->entries[i];
          it->key        = entry.key;
          it->value      = entry.value;
          return true;
        }
    }
  return false;
}

static void print_entry(const char* key, void* value)
{ printf("%s = %p\n", key, value); }

void hm_debug(hmi* it) { hm_foreach(it, print_entry); }

void hm_foreach(hmi* it, void (*ptr)(const char* key, void*))
{
  while (hm_next(it)) { ptr(it->key, it->value); }
}
