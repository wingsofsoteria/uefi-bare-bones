#pragma once
#include "types.h"

#include <host.h>
aml_ptr_t* create_ptr(void* data, uint8_t type);
/// locate_object:
/// ns: the current namespace
/// static_key: name of the object to search for
/// len: length of static_key
/// namespace: should the return type be aml_namespace_t* or aml_ptr_t*
void* locate_object(
  aml_namespace_t* ns,
  char*            static_key,
  size_t           len,
  bool             namespace
);
// aml_ptr_t*    locate_object(aml_namespace_t* ns, aml_name_t key);
void          debug_exit();
uint64_t      read_mem(void* address, uint8_t access_len);
uint64_t      read_io(uint16_t port, uint8_t access_len);
size_t        parse_length(aml_namespace_t* ns);
aml_buffer_t* term_arg_to_buffer(aml_namespace_t* ns);
uint64_t      term_arg_to_int(aml_namespace_t* ns);
void          parse_next(aml_namespace_t* ns);
void parse_termlist(aml_namespace_t* ns, uint8_t* start, const uint8_t* end);
// aml_namespace_t* get_scope(aml_namespace_t* current, aml_name_t key);
size_t parse_next_field_elem(
  aml_namespace_t* ns,
  int              offset,
  aml_field_t*     parent
);

/// checks every char in a namestring to make sure it contains valid bytes since
/// we use parse_namestring before checking other types
bool valid_name(aml_name_t* name);
void parse_data_object(aml_namespace_t* ns, aml_variable_t* var);
