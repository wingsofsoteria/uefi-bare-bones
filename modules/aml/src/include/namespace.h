#pragma once
#include "types.h"
aml_namespace_t* create_namespace(
  aml_namespace_t* parent,
  hash_key         name,
  uint8_t*         code
);
// aml_namespace_t* locate_namespace(aml_namespace_t* parent, aml_name_t key);
void add_child_to_namespace(aml_namespace_t* ns, hash_key key, aml_ptr_t* data);
void debug_namespace(const char*, void* ptr);
aml_namespace_t* root();
