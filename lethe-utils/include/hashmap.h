#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#define KEY_LEN 4
typedef char      hash_key[KEY_LEN];
typedef struct hm hm;

typedef struct
{
  const char* key;
  void*       value;
  hm*         _map;
  size_t      _index;
} hmi;

hm*         hm_create();
void*       hm_get(hm*, const char*);
const char* hm_set(hm*, const char*, void*);
hmi         hm_iter(hm*);
bool        hm_next(hmi*);
void        hm_debug(hmi*);
void        hm_foreach(hmi*, void (*)(const char*, void*));
