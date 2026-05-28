#include "aml.h"
#include "helpers.h"
#include "host.h"
#include "namespace.h"
#include "types.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

aml_ptr_t* get_object(char* key)
{
  unimplemented(key[0] != '\\');
  memmove(key, key + 1, 4);
  size_t len = strlen(key);
  if (len == 4) { return hash_map_get(root()->children, key, NULL); }
  char* scope_name = malloc(len - 4);
  memcpy(scope_name, key, len - 4);
  aml_name_t       scope_key = { len, scope_name };
  aml_namespace_t* scope     = get_scope(root(), scope_key);
  free(scope_name);
  scope_key = (aml_name_t){ 0 };
  assert(scope);
  memmove(key, key + len - 4, 4);
  return hash_map_get(scope->children, key, NULL);
}

void execute_method(aml_method_t* method) {}
