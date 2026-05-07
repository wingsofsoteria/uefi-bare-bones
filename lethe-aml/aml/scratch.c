/*
 * method: (name, code) should be stored within a namespace (scope, device, etc)
 * */

#include "aml.h"
#include "hashmap.h"
#include "host.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct aml_namespace
{
  struct aml_namespace* parent;
  char*                 name;
  uint8_t*              code;
  hash_map_t*           children;
} aml_namespace_t;

typedef struct
{
  char inner[1022];
} aml_name_t;

typedef struct
{
  aml_name_t label;
  void*      data;
} aml_variable_t;

typedef struct
{
  size_t   buffer_size;
  uint8_t* buffer;
} aml_buffer_t;

typedef struct
{
  aml_name_t method_name;
  uint8_t    method_flags;
  size_t     method_len;
  uint8_t*   code;
} aml_method;

static aml_namespace_t* _root = NULL;

static aml_namespace_t* create_namespace(
  aml_namespace_t* parent,
  char*            name,
  uint8_t*         code
)
{
  aml_namespace_t* namespace = malloc(sizeof(aml_namespace_t));
  if (!namespace) return NULL;
  namespace->parent = parent;
  namespace->code   = code;
  namespace->name   = name;
  hash_map_t* map;
  hash_map_create(&map, 5);
  namespace->children = map;
  return namespace;
}

static int init_aml_namespaces(uint8_t* root_code)
{
  aml_namespace_t* root_ns = create_namespace(NULL, "\\___", root_code);
  if (!root_ns) return 1;
  _root = root_ns;
  return 0;
}

static size_t parse_length(aml_namespace_t* ns)
{
  uint8_t lead_byte  = *ns->code++;
  uint8_t byte_count = (lead_byte >> 6) & 0b11;
  size_t  length     = lead_byte & 0x3F;
  switch (byte_count)
    {
      case 0:
        {
          return length;
        }
      case 1:
        {
          length |= ((*ns->code++) << 4);
          return length;
        }
      case 2:
        {
          length |= ((*ns->code++) << 4);
          length |= ((*ns->code++) << 12);
          return length;
        }
      default:
        {
          assert(byte_count == 3);
          length |= ((*ns->code++) << 4);
          length |= ((*ns->code++) << 12);
          length |= ((*ns->code++) << 20);
          return length;
        }
    }

  return 0;
}

static uint64_t term_arg_to_int(aml_namespace_t* ns)
{
  uint8_t op = *ns->code++;
  switch (op)
    {
      case ZERO_OP:
        {
          return 0;
        }
      case ONE_OP:
        {
          return 1;
        }
      case ONES_OP:
        {
          return ~0UL;
        }
      case BYTE_PREFIX:
        {
          return *ns->code++;
        }
      case WORD_PREFIX:
        {
          uint16_t out = ns->code[0] | (uint16_t)ns->code[1] << 8;
          ns->code    += 2;
          return out;
        }
      case DWORD_PREFIX:
        {
          uint32_t out = ns->code[0] | (uint32_t)ns->code[1] << 8 |
                         (uint32_t)ns->code[2] << 16 |
                         (uint32_t)ns->code[3] << 24;
          ns->code    += 4;
          return out;
        }
      default:
        {
          ns->code--;
          char name[4] = { ns->code[0], ns->code[1], ns->code[2], ns->code[3] };
          int  index   = -1;
          aml_variable_t* var = hash_map_get(ns->children, name, &index);
          if (index == -1)
            {
              AML_EXIT();
              return 1;
            }
          ns->code += 4;
          return *(uint64_t*)var->data;
        }
    }
}

static void parse_termlist(aml_namespace_t* /*ns*/, const uint8_t* /*end*/);

static void def_if_else(aml_namespace_t* ns)
{
  uint8_t* code_copy = ns->code;
  size_t   if_len    = parse_length(ns);
  uint64_t predicate = term_arg_to_int(ns);
  if (predicate) { parse_termlist(ns, code_copy + if_len); }
  else
    {
      ns->code = code_copy + if_len;
    }
  if (*ns->code == ELSE_OP)
    {
      ns->code++;
      code_copy       = ns->code;
      size_t else_len = parse_length(ns);
      if (!predicate) { parse_termlist(ns, code_copy + else_len); }
      ns->code = code_copy + if_len;
    }
}

static void parse_namestring(aml_namespace_t* ns, aml_name_t* name)
{
  uint8_t* copy     = ns->code;
  uint8_t* name_ptr = (uint8_t*)name->inner;
  while (*copy == '\\' || *copy == '^') { *name_ptr++ = *copy++; }
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

  uint32_t char_count = seg_count * 4;
  while (char_count--) { *name_ptr++ = *copy++; }
  *name_ptr = '\0';
  ns->code  = copy;
}

static void def_method(aml_namespace_t* ns)
{
  uint8_t*   code_copy  = ns->code;
  size_t     method_len = parse_length(ns);
  aml_name_t method_name;
  parse_namestring(ns, &method_name);
  uint8_t     method_flags = *ns->code++;
  aml_method* method       = malloc(sizeof(aml_method));
  method->method_flags     = method_flags;
  method->method_len       = method_len - (ns->code - code_copy);
  method->method_name      = method_name;
  method->code             = ns->code;
  hash_map_push(ns->children, method_name.inner, method, sizeof(aml_method));
  ns->code = code_copy + method_len;
}

static void parse_data_object(aml_namespace_t* ns, void* out_data)
{
  uint8_t op = *ns->code++;
  switch (op)
    {
      case ZERO_OP:
        {
          *(uint64_t*)out_data = 0;
          break;
        }
      case ONE_OP:
        {
          *(uint64_t*)out_data = 1;
          break;
        }
      case ONES_OP:
        {
          *(uint64_t*)out_data = 0xFF;
          break;
        }
      case BYTE_PREFIX:
        {
          uint64_t data        = *ns->code++;
          *(uint64_t*)out_data = data;
          break;
        }
      case WORD_PREFIX:
        {
          uint8_t  lower       = *ns->code++;
          uint8_t  upper       = *ns->code++;
          uint16_t word        = (uint16_t)upper << 8 | lower;
          *(uint64_t*)out_data = word;
          break;
        }
      case DWORD_PREFIX:
        {
          *(uint64_t*)out_data = ns->code[0] | (uint32_t)ns->code[1] << 8 |
                                 (uint32_t)ns->code[2] << 16 |
                                 (uint32_t)ns->code[3] << 24;
          ns->code            += 4;
          break;
        }
      case STRING_PREFIX:
        {
          int i = 0;
          while (ns->code[i] != 0) { i++; }
          i++;
          free(out_data);
          char* ptr = malloc(i * sizeof(char));
          for (int j = 0; j < i; j++) { ptr[j] = *ns->code++; }
          out_data = ptr;
          break;
        }
      case PACKAGE_OP:
        {
          uint8_t*      code_copy    = ns->code;
          size_t        pkg_len      = parse_length(ns);
          uint8_t       num_elements = *ns->code++;
          aml_buffer_t* ptr          = out_data;
          ptr->buffer                = ns->code;
          ptr->buffer_size           = num_elements;
          ns->code                   = code_copy + pkg_len;
          break;
        }
      case BUFFER_OP:
        {
          size_t   buf_len     = parse_length(ns);
          uint64_t buffer_size = term_arg_to_int(ns);
          uint8_t* buffer      = malloc(buffer_size * sizeof(uint8_t));
          memcpy(buffer, ns->code, buffer_size);
          aml_buffer_t* ptr = out_data;
          ptr->buffer_size  = buffer_size;
          ptr->buffer       = buffer;
          break;
        }
      default:
        {
          AML_EXIT();
          break;
        }
    }
}

static void def_name(aml_namespace_t* ns)
{
  aml_name_t name;
  parse_namestring(ns, &name);
  void* data = malloc(sizeof(void*));
  parse_data_object(ns, data);
  aml_variable_t* name_var = malloc(sizeof(aml_variable_t));
  name_var->data           = data;
  name_var->label          = name;
  hash_map_push(ns->children, name.inner, name_var, sizeof(name_var));
  hash_map_debug(ns->children);
}

static void def_op_region(aml_namespace_t* ns)
{
  aml_name_t name;
  parse_namestring(ns, &name);
  uint8_t  region_space  = *ns->code++;
  uint64_t region_offset = term_arg_to_int(ns);
  uint64_t region_len    = term_arg_to_int(ns);
}

static void def_field(aml_namespace_t* ns)
{
  uint8_t*   code_copy = ns->code;
  size_t     field_len = parse_length(ns);
  aml_name_t name;
  parse_namestring(ns, &name);
  uint8_t flags = *ns->code++;
  // TODO parse field list
  ns->code = code_copy + field_len;
}

static void def_index_field(aml_namespace_t* ns)
{
  uint8_t*   code_copy       = ns->code;
  size_t     index_field_len = parse_length(ns);
  aml_name_t name;
  parse_namestring(ns, &name);
  parse_namestring(ns, &name);
  uint8_t flags = *ns->code++;
  ns->code      = code_copy + index_field_len;
}

static void parse_next(aml_namespace_t* ns)
{
  uint16_t op = 0;
  if (*ns->code == EXT_OP_PREFIX)
    {
      ns->code++;
      op = (EXT_OP_PREFIX << 8) | *ns->code++;
    }
  else
    {
      op = *ns->code++;
    }
  switch (op)
    {
      case IF_OP:
        {
          def_if_else(ns);
          break;
        }
      case NAME_OP:
        {
          def_name(ns);
          break;
        }
      case METHOD_OP:
        {
          def_method(ns);
          break;
        }
      case (EXT_OP_PREFIX << 8) | EXT_OP_REGION_OP:
        {
          def_op_region(ns);
          break;
        }
      case (EXT_OP_PREFIX << 8) | EXT_FIELD_OP:
        {
          def_field(ns);
          break;
        }
      case (EXT_OP_PREFIX << 8) | EXT_INDEX_FIELD_OP:
        {
          def_index_field(ns);
          break;
        }
      default:
        {
          printf("%x\n", op);
          AML_EXIT();
          break;
        }
    }
}

static void parse_termlist(aml_namespace_t* ns, const uint8_t* end)
{
  while (ns->code < end) { parse_next(ns); }
}

static void populate_children(aml_namespace_t* ns, size_t table_len)
{ parse_termlist(ns, ns->code + table_len); }

void parse_table(acpi_aml_table_t* table)
{
  if (!_root)
    {
      if (init_aml_namespaces(table->definition_blocks)) { return; }
    }
  populate_children(_root, table->length - 36);
}
