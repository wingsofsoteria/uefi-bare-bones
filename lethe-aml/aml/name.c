#include "name.h"

#include "helpers.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void prepend_str(aml_name_t* name, char* str, int len)
{
  debug_exit();
  int offset = len;
  // assert((len + name->count) < MAX_CHARS);
  // if (name->count > 0) { memmove(name->inner + len, name->inner,
  // name->count); }
  memcpy(name->inner, str, len);
  // name->count += len;
}

aml_name_t resolve_name(aml_namespace_t* ns, aml_name_t key)
{
  debug_exit();
  alog("%s %d\n", key.inner, key.count);
  unimplemented(key.inner[0] == '^' || key.inner[0] == '\\');
  aml_name_t resolved_key = { 0, {} };
  // if (key.count > 4 && key.count % 4 == 0)
  //   {
  //     prepend_str(&resolved_key, key.inner, key.count);
  //     prepend_str(&resolved_key, "\\", 1);
  //     printf("%s -> %s\n", key.inner, resolved_key.inner);
  //     return resolved_key;
  //   }
  // prepend_str(&resolved_key, key.inner, key.count);
  prepend_str(&resolved_key, ".", 1);
  prepend_str(&resolved_key, ns->name, strlen(ns->name));
  return resolved_key;
}

aml_name_t* parse_namestring(aml_namespace_t* ns)
{
  aml_name_t* name        = malloc(sizeof(aml_name_t));
  uint8_t*    copy        = ns->code;
  char        char_prefix = 0;
  size_t      len         = 0;
  while (*copy == '\\' || *copy == '^')
    {
      if (!char_prefix)
        {
          char_prefix = *copy;
          len         = 1;
        }
      copy++;
    }
  uint8_t prefix    = *copy++;
  uint8_t seg_count = 0;
  if (prefix == 0x2E) { seg_count = 2; }
  else if (prefix == 0x2F) { seg_count = *copy++; }
  else if (prefix == 0x0) { seg_count = 0; }
  else
    {
      copy--;
      seg_count = 1;
    }
  len        += (seg_count * KEY_LEN);
  name->inner = malloc(len * sizeof(char));
  name->count = len;
  if (char_prefix)
    {
      name->inner[0] = char_prefix;
      memcpy(name->inner + 1, copy, seg_count * KEY_LEN);
    }
  else
    {
      memcpy(name->inner, copy, seg_count * KEY_LEN);
    }
  copy    += seg_count * KEY_LEN;
  ns->code = copy;
  return name;
}

aml_name_t* trim_name(aml_name_t* name)
{
  if (name->count <= KEY_LEN) { return name; }
  alog("%s -> ", name->inner);
  char* inner = malloc(KEY_LEN);
  memcpy(inner, name->inner + name->count - KEY_LEN, KEY_LEN);
  free(name->inner);
  name->inner = inner;
  name->count = KEY_LEN;
  printf("%s\n", name->inner);
  return name;
}
