#include "defs.h"

#include "aml.h"
#include "hashmap.h"
#include "helpers.h"
#include "host.h"
#include "name.h"
#include "namespace.h"
#include "opcodes.h"
#include "tables.h"
#include "types.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void def_if_else(aml_namespace_t* ns)
{
  uint8_t* code_copy = ns->code;
  size_t   if_len    = parse_length(ns);
  uint64_t predicate = term_arg_to_int(ns);
  if (predicate) { parse_termlist(ns, code_copy, code_copy + if_len); }
  else
    {
      ns->code = code_copy + if_len;
    }
  if (*ns->code == ELSE_OP)
    {
      ns->code++;
      code_copy       = ns->code;
      size_t else_len = parse_length(ns);
      if (!predicate) { parse_termlist(ns, code_copy, code_copy + else_len); }
      ns->code = code_copy + if_len;
    }
}

void def_method(aml_namespace_t* ns)
{
  uint8_t*      code_copy    = ns->code;
  size_t        method_len   = parse_length(ns);
  aml_name_t*   method_name  = parse_namestring(ns);
  uint8_t       method_flags = *ns->code++;
  aml_method_t* method       = malloc(sizeof(aml_method_t));
  method->flags              = method_flags;
  method->len                = method_len - (ns->code - code_copy);
  method->code               = ns->code;
  memcpy(
    method->name,
    method_name->inner + method_name->count - KEY_LEN,
    KEY_LEN
  );
  method->scope          = create_namespace(NULL, method->name, method->code);
  aml_namespace_t* scope = NULL;
  if (method_name->count == KEY_LEN || method_name->count == KEY_LEN + 1)
    {
      switch (method_name->inner[0])
        {
          case '\\':
            {
              scope = root();
              break;
            }
          case '^':
            {
              scope = ns->parent;
              break;
            }
          default:
            {
              scope = ns;
              break;
            }
        }
    }
  else
    {
      size_t scope_len  = method_name->count - KEY_LEN;
      char*  scope_name = malloc(scope_len);
      if (!scope_name) { debug_exit(); }
      memcpy(scope_name, method_name->inner, scope_len);
      scope = locate_object(ns, scope_name, scope_len, true);
    }
  if (!scope) { debug_exit(); }
  add_child_to_namespace(scope, method->name, create_ptr(method, TYPE_METHOD));
  method->scope->parent = scope;
  free(method_name->inner);
  free(method_name);
  ns->code = code_copy + method_len;
}

void def_name(aml_namespace_t* ns)
{
  aml_name_t* name = parse_namestring(ns);
  unimplemented(name->count != KEY_LEN);
  aml_variable_t* name_var = malloc(sizeof(aml_variable_t));
  parse_data_object(ns, name_var);
  memcpy(name_var->label, name->inner + name->count - KEY_LEN, KEY_LEN);
  free(name->inner);
  free(name);
  add_child_to_namespace(ns, name_var->label, create_ptr(name_var, TYPE_NAME));
}

void def_op_region(aml_namespace_t* ns)
{
  aml_name_t* name = parse_namestring(ns);
  unimplemented(name->count != KEY_LEN);
  uint8_t                 region_space  = *ns->code++;
  uint64_t                region_offset = term_arg_to_int(ns);
  uint64_t                region_len    = term_arg_to_int(ns);
  aml_operation_region_t* region = malloc(sizeof(aml_operation_region_t));
  region->region_space           = region_space;
  region->len                    = region_len;
  region->offset                 = region_offset;
  memcpy(region->name, name->inner + name->count - KEY_LEN, KEY_LEN);
  add_child_to_namespace(ns, region->name, create_ptr(region, TYPE_REGION));
  free(name->inner);
  free(name);
}

void def_field(aml_namespace_t* ns)
{
  uint8_t*    code_copy = ns->code;
  size_t      field_len = parse_length(ns);
  aml_name_t* name      = parse_namestring(ns);

  aml_ptr_t*   obj   = locate_object(ns, name->inner, name->count, false);
  uint8_t      flags = *ns->code++;
  aml_field_t* field = malloc(sizeof(aml_field_t));
  field->access_type = flags & 0xF;
  field->should_lock = (flags & 0x10) != 0;
  field->update_rule = (flags & 0xC0) >> 6;
  if (obj == NULL) { debug_exit(); }
  if (obj->type != TYPE_REGION) { debug_exit(); }
  field->region = obj->data;
  assert((flags & 0x80) == 0);
  int offset = 0;
  while (1)
    {
      size_t len = parse_next_field_elem(ns, offset, field);
      if (len == UINT64_MAX) { break; }
      offset += len;
    }
  ns->code = code_copy + field_len;
  free(name->inner);
  free(name);
}

void def_index_field(aml_namespace_t* ns)
{
  uint8_t* code_copy       = ns->code;
  size_t   index_field_len = parse_length(ns);
  // aml_name_t name;
  // parse_namestring(ns, &name);
  // parse_namestring(ns, &name);
  // uint8_t flags = *ns->code++;
  ns->code = code_copy + index_field_len;
}

void def_scope(aml_namespace_t* ns)
{
  uint8_t*    code_copy  = ns->code;
  size_t      scope_len  = parse_length(ns);
  uint8_t*    end        = code_copy + scope_len;
  aml_name_t* scope_name = parse_namestring(ns);
  if (!scope_name->count)
    {
      parse_termlist(root(), ns->code, end);
      ns->code = end;
      return;
    }

  aml_namespace_t* scope =
    locate_object(ns, scope_name->inner, scope_name->count, true);
  if (scope == NULL)
    {
      aml_log("could not find scope %s\n", scope_name->inner);
      if (scope_name->count > KEY_LEN)
        {
          char  parent[KEY_LEN];
          char* self = calloc(scope_name->count - KEY_LEN, sizeof(char));
          memcpy(parent, scope_name->inner + 1, KEY_LEN);
          memcpy(
            self,
            scope_name->inner + KEY_LEN + 1,
            scope_name->count - KEY_LEN
          );
          aml_log("%s %s\n", parent, self);
          aml_namespace_t* parent_ns = locate_object(ns, parent, KEY_LEN, true);
        }
      debug_exit();
      // scope      = create_namespace(parent, scope_name.inner, ns->code, 0,
      // 0);
    }
  parse_termlist(scope, ns->code, end);
  ns->code = end;
}

void def_device(aml_namespace_t* ns)
{
  uint8_t*         code_copy   = ns->code;
  size_t           device_len  = parse_length(ns);
  uint8_t*         end         = code_copy + device_len;
  aml_name_t*      device_name = parse_namestring(ns);
  aml_namespace_t* device =
    locate_object(ns, device_name->inner, device_name->count, true);

  if (!device)
    {
      aml_namespace_t* parent = ns;
      if (device_name->count > KEY_LEN)
        {
          size_t len = device_name->count - KEY_LEN;
          char*  key = malloc(len);
          memcpy(key, device_name->inner, len);
          parent      = locate_object(ns, key, len, true);
          device_name = trim_name(device_name);
        }
      device = create_namespace(parent, device_name->inner, ns->code);
    }

  parse_termlist(device, ns->code, end);
  ns->code = end;
}

void def_alias(aml_namespace_t* ns)
{
  aml_name_t* source = parse_namestring(ns);
  aml_name_t* alias  = parse_namestring(ns);
  unimplemented(source->count != KEY_LEN);
  unimplemented(alias->count != KEY_LEN);
  aml_ptr_t* ref = hm_get(ns->children, source->inner);
  if (ref == NULL) { debug_exit(); }
  add_child_to_namespace(ns, alias->inner, ref);
  free(alias->inner);
  free(source->inner);
  free(alias);
  free(source);
}

void def_mutex(aml_namespace_t* ns)
{
  aml_name_t*  mutex_name = parse_namestring(ns);
  aml_mutex_t* mutex      = malloc(sizeof(aml_mutex_t));
  unimplemented(mutex_name->count != KEY_LEN);
  memcpy(mutex->name, mutex_name + mutex_name->count - KEY_LEN, KEY_LEN);
  mutex->sync_flags = *ns->code++;
  add_child_to_namespace(ns, mutex->name, create_ptr(mutex, TYPE_MUTEX));
}

void def_create_word_field(aml_namespace_t* ns)
{
  aml_buffer_t* buffer     = term_arg_to_buffer(ns);
  uint64_t      byte_index = term_arg_to_int(ns);
  aml_name_t*   name       = parse_namestring(ns);
  unimplemented(name->count != KEY_LEN);
  aml_word_field_t* word_field = malloc(sizeof(aml_word_field_t));
  memcpy(word_field->name, name->inner, KEY_LEN);
  word_field->field = (uint16_t*)((char*)buffer->buffer + byte_index);
  add_child_to_namespace(
    ns,
    word_field->name,
    create_ptr(word_field, TYPE_WORD_FIELD)
  );
}

static void populate_children(aml_namespace_t* ns, size_t table_len)
{ parse_termlist(ns, ns->code, ns->code + table_len); }

void parse_table(acpi_aml_t* table)
{
  root()->code = table->data;
  populate_children(root(), table->header.length - 36);
}
