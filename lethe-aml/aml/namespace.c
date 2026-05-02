#include "aml.h"
#include "fnv.h"
#include "host.h"
#include "stdlib.h"

struct MapNode
{
  int         __flag;
  const char* key;
  void*       __data;
};

struct Map
{
  const char*     key;
  int             capacity;
  int             count;
  struct MapNode* __inner;
};

static struct Map* __root = NULL;

void* new_map(const char key[4], int capacity)
{
  struct Map* map = malloc(sizeof(struct Map));
  map->__inner    = calloc(capacity, sizeof(struct MapNode));
  map->key        = key;
  map->capacity   = capacity;
  map->count      = 0;
  return map;
}

void append(void* map_ptr, const char key[4], void* value)
{
  struct Map*    map  = (struct Map*)map_ptr;
  int            slot = fnv_32a_buf(key, 4); // turn key into slot?
  struct MapNode node = { 1, key, value };

  if (map->count >= map->capacity)
    {
      map->capacity *= 2;
      void* ptr = realloc(map->__inner, map->capacity * sizeof(struct MapNode));
      if (ptr == NULL)
        {
          AML_LOG("Failed to resize map");
          AML_EXIT();
        }
      map->__inner = ptr;
    }
  if (map->__inner[slot].__flag == 1)
    {
      AML_LOG("Collision in Node %.4s with key %.4s\n", map->key, node.key);
      AML_EXIT();
    }
  map->__inner[slot] = node;
}

void init_map() { __root = new_map("\\", 10); }

void* root() { return __root; }
