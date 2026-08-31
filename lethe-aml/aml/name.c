#include "name.h"

#include "hashmap.h"
#include "host.h"
#include "namespace.h"
#include "types.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

aml_name_t resolve_name(aml_namespace_t* ns, aml_name_t key)
{
  unimplemented(key.inner[0] == '^');
  if (ns == root() || key.inner[0] == '\\')
    {
      char* copy = malloc(key.count * sizeof(char));
      memcpy(copy, key.inner, key.count);
      return (aml_name_t){ .count = key.count, .inner = copy };
    }
  int levels = 0;
  // first loop is to calculate how many parent namespaces exist between `ns`
  // and `root()`
  aml_namespace_t* current = ns;
  while (current->parent)
    {
      levels++;
      current = current->parent;
    }
  // sanity checks
  assert(current == root());
  assert(levels > 0);
  // each namespace only has KEY_LEN chars + the name we are resolving + 1 for
  // the root char
  size_t size = levels * KEY_LEN * sizeof(char);
  size       += key.count + 1;

  char* absolute_name = malloc(size);
  // second loop is to prepend each namespace's name to the final string
  int offset = size - key.count;
  memcpy(absolute_name + offset, key.inner, key.count);
  offset -= KEY_LEN;
  current = ns;
  while (current->parent)
    {
      memcpy(absolute_name + offset, current->name, KEY_LEN);
      offset -= KEY_LEN;
      current = current->parent;
    }
  absolute_name[0] = '\\';
  aml_log("%s\n", absolute_name);
  return (aml_name_t){ .count = size, .inner = absolute_name };
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
  aml_log("%s -> ", name->inner);
  char* inner = malloc(KEY_LEN);
  memcpy(inner, name->inner + name->count - KEY_LEN, KEY_LEN);
  free(name->inner);
  name->inner = inner;
  name->count = KEY_LEN;
  printf("%s\n", name->inner);
  return name;
}
