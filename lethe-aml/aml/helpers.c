#include "helpers.h"

#include "defs.h"
#include "hashmap.h"
#include "host.h"
#include "name.h"
#include "namespace.h"
#include "opcodes.h"
#include "types.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG

aml_ptr_t* create_ptr(void* data, uint8_t type)
{
  aml_ptr_t* ptr = malloc(sizeof(aml_ptr_t));
  ptr->data      = data;
  ptr->type      = type;
  return ptr;
}

static void* stage3(
  aml_namespace_t* ns,
  char*            static_key,
  size_t           len,
  bool             namespace
)
{
  aml_namespace_t* current = ns;
  void*            ptr     = NULL;
  do
    {
      ptr = namespace ? hm_get(current->namespaces, static_key)
                      : hm_get(current->children, static_key);
      if (ptr) { return ptr; }
      current = current->parent;
      if (!current) { return NULL; }
    }
  while (true);
}

// this will ALWAYS do the search relative to ns
static aml_namespace_t* resolve_scope(
  aml_namespace_t* ns,
  char*            static_key,
  size_t           len
)
{
  if (len % KEY_LEN != 0) return NULL;
  size_t           rem = len;
  char             key[KEY_LEN];
  size_t           i       = 0;
  aml_namespace_t* next_ns = ns;
  while (rem)
    {
      memcpy(key, static_key + i, KEY_LEN);
      void* ptr = hm_get(next_ns->namespaces, key);
      if (!ptr) { return NULL; }
      next_ns = ptr;
      rem    -= KEY_LEN;
      i      += KEY_LEN;
    }

  if (!next_ns || next_ns == ns) { return NULL; }
  return next_ns;
}

static void* stage2(
  aml_namespace_t* ns,
  char*            static_key,
  size_t           len,
  bool             namespace
)
{
  char key[KEY_LEN] = { 0 };
  if (len == KEY_LEN)
    {
      memcpy(key, static_key, KEY_LEN);
      return namespace ? hm_get(ns->namespaces, key)
                       : hm_get(ns->children, key);
    }
  size_t scope_len = len - KEY_LEN;
  char*  scope_key = malloc(scope_len);
  if (!scope_key) { return NULL; }
  memcpy(scope_key, static_key, scope_len);
  memcpy(key, static_key + scope_len, KEY_LEN);

#ifdef DEBUG
  aml_log("%s %s %s %d\n", key, scope_key, static_key, len);
#endif
  aml_namespace_t* scope = resolve_scope(ns, scope_key, scope_len);
  free(scope_key);
  if (!scope) { return NULL; }
  return namespace ? hm_get(scope->namespaces, key)
                   : hm_get(scope->children, key);
}

void* locate_object(
  aml_namespace_t* ns,
  char*            static_key,
  size_t           len,
  bool             namespace
)
{
  aml_name_t test_name = (aml_name_t){ len, static_key };
  if (len == 1) { return root(); }
  if (!valid_name(&test_name))
    {
      aml_log("Key is not valid");
      debug_exit();
    }
  if (len == KEY_LEN) { return stage3(ns, static_key, len, namespace); }

  aml_namespace_t* scope = NULL;
  switch (static_key[0])
    {
      case '^':
        {
          if (!ns->parent) { return NULL; }
          scope = ns->parent;
          len  -= 1;
          memmove(static_key, static_key + 1, len);
          break;
        }
      case '\\':
        {
          scope = root();
          len  -= 1;
          memmove(static_key, static_key + 1, len);
          break;
        }
      default:
        {
          scope = ns;
          break;
        }
    }

#ifdef DEBUG
  aml_log(
    "Searching: %.4s for key %.*s with len %d\n",
    scope->name,
    test_name.count,
    test_name.inner,
    test_name.count
  );
#endif
  return stage2(scope, static_key, len, namespace);
}

void debug_exit()
{
  debug_namespace("\\___", root());

  AML_EXIT();
}

#ifdef __is_libk
uint64_t read_mem(void* address, uint8_t access_len)
{
  switch (access_len)
    {
      case 8:
        {
          return *(volatile uint8_t*)(address);
        }
      case 16:
        {
          return *(volatile uint16_t*)(address);
        }
      case 32:
        {
          return *(volatile uint32_t*)(address);
        }
      case 64:
        {
          return *(volatile uint64_t*)(address);
        }
      default:
        {
          debug_exit();
        }
    }
  return 0;
}
#else
uint64_t read_mem(void* address, uint8_t access_len)
{ return address && access_len | UINT64_MAX; }
#endif

uint64_t read_io(uint16_t port, uint8_t access_len)
{
  switch (access_len)
    {
      case 8:
        {
          return inb(port);
        }
      case 16:
        {
          return inw(port);
        }
      case 32:
        {
          return ind(port);
        }
      default:
        {
          debug_exit();
        }
    }
  return 0;
}

size_t parse_length(aml_namespace_t* ns)
{
  uint8_t lead_byte  = *ns->code++;
  uint8_t byte_count = lead_byte >> 6 & 0b11;
  size_t  length     = lead_byte & 0x3F;
  switch (byte_count)
    {
      case 0:
        {
          return length;
        }
      case 1:
        {
          length |= *ns->code++ << 4;
          return length;
        }
      case 2:
        {
          length |= *ns->code++ << 4;
          length |= *ns->code++ << 12;
          return length;
        }
      default:
        {
          if (byte_count != 3) { break; }
          assert(byte_count == 3);
          length |= ((*ns->code++) << 4);
          length |= ((*ns->code++) << 12);
          length |= ((*ns->code++) << 20);
          return length;
        }
    }

  return 0;
}

aml_buffer_t* term_arg_to_buffer(aml_namespace_t* ns)
{
  // assume op is for a name segment
  hash_key name;
  memcpy(name, ns->code, KEY_LEN);
  aml_ptr_t* obj = locate_object(ns, name, KEY_LEN, false);
  switch (obj->type)
    {
      case TYPE_NAME:
        {
          aml_variable_t* var = obj->data;
          if (var->data_type == DATA_BUF)
            {
              ns->code += KEY_LEN;
              return var->buffer;
            }
#ifdef DEBUG
          aml_log("var->data_type = %d\n", var->data_type);
#endif
          debug_exit();
          break;
        }
      default:
        {
#ifdef DEBUG
          aml_log("%p\n%d\n", obj->data, obj->type);
#endif
          debug_exit();
          break;
        }
    }
  return NULL;
}

uint64_t term_arg_to_int(aml_namespace_t* ns)
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
      case LNOT_OP:
        {
          uint64_t predicate = term_arg_to_int(ns);
          return !predicate;
        }
      case LEQUAL_OP:
        {
          uint64_t term1 = term_arg_to_int(ns);
          uint64_t term2 = term_arg_to_int(ns);
          return term1 == term2;
        }
      case LAND_OP:
        {
          uint64_t term1 = term_arg_to_int(ns);
          uint64_t term2 = term_arg_to_int(ns);
          return term1 && term2;
        }
      case LLESS_OP:
        {
          uint64_t term1 = term_arg_to_int(ns);
          uint64_t term2 = term_arg_to_int(ns);
          return term1 < term2;
        }
      case LGREATER_OP:
        {
          uint64_t term1 = term_arg_to_int(ns);
          uint64_t term2 = term_arg_to_int(ns);
          return term1 > term2;
        }
      default:
        {
          ns->code--;
          hash_key name;
          memcpy(name, ns->code, KEY_LEN);
          aml_ptr_t* obj = locate_object(ns, name, KEY_LEN, false);
          if (!obj) { debug_exit(); }
          void*   ptr  = obj->data;
          uint8_t type = obj->type;

          if (ptr == NULL)
            {
              debug_exit();
              return 1;
            }

          ns->code += 4;
          if (type == TYPE_NAME)
            {
              aml_variable_t* data = ptr;
              switch (data->data_type)
                {
                  case DATA_BYTE:
                    {
                      return data->byte;
                    }
                  case DATA_SHORT:
                    {
                      return data->short_int;
                    }
                  case DATA_INT:
                    {
                      return data->integer;
                    }
                  default:
                    {
                      debug_exit();
                      return 1;
                    }
                }
            }
          else if (type == TYPE_FIELD_OFFSET)
            {

              aml_named_field_t*      data     = ptr;
              aml_operation_region_t* region   = data->parent->region;
              int                     read_len = 0;
              switch (data->parent->access_type)
                {
                  case AnyAcc:
                  case ByteAcc:
                    {
                      read_len = 8;
                      break;
                    }
                  case WordAcc:
                    {
                      read_len = 16;
                      break;
                    }
                  case DWordAcc:
                    {
                      read_len = 32;
                      break;
                    }
                  case QWordAcc:
                    {
                      read_len = 64;
                      break;
                    }
                  default:
                    {
                      debug_exit();
                    }
                }
              switch (region->region_space)
                {
                  case 0:
                    {
                      return read_mem(
                        (void*)(region->offset + data->offset),
                        read_len
                      );
                    }
                  case 1:
                    {
                      return read_io(region->offset + data->offset, read_len);
                    }
                  default:
                    {
                      debug_exit();
                    }
                }
              return read_io(region->offset + data->offset, read_len);
            }
          return 0;
        }
    }
}

void parse_next(aml_namespace_t* ns)
{
  uint16_t op = 0;
  if (*ns->code == EXT_OP_PREFIX)
    {
      ns->code++;
      op = EXT_OP_PREFIX << 8 | *ns->code++;
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
      case SCOPE_OP:
        {
          def_scope(ns);
          break;
        }
      case (EXT_OP_PREFIX << 8) | EXT_DEVICE_OP:
        {
          def_device(ns);
          break;
        }
      case (EXT_OP_PREFIX << 8) | EXT_MUTEX_OP:
        {
          def_mutex(ns);
          break;
        }
      case ALIAS_OP:
        {
          def_alias(ns);
          break;
        }
      case CREATE_WORDFIELD_OP:
        {
          def_create_word_field(ns);
        }
      case (EXT_OP_PREFIX << 8) | 0x83:
        {
          uint8_t* copy = ns->code;
          size_t   len  = parse_length(ns);
          ns->code      = copy + len;
          break;
        }
      default:
        {
#ifdef DEBUG
          aml_log("%x ", op);
          debug_code(ns, 5);
#endif
          debug_exit();
          break;
        }
    }
}

void parse_termlist(aml_namespace_t* ns, uint8_t* start, const uint8_t* end)
{
  uint8_t* copy = ns->code;
  ns->code      = start;
  while (ns->code < end) { parse_next(ns); }
  ns->code = copy;
}

size_t parse_next_field_elem(
  aml_namespace_t* ns,
  int              offset,
  aml_field_t*     parent
)
{
  uint8_t* code_copy = ns->code;
  uint8_t  op        = *ns->code++;
  switch (op)
    {
      case Reserved:
        {
          // out_field->reserved_field.len = parse_length(ns);
          return parse_length(ns);
          break;
        }
      case Access:
        {
          // out_field->type                = Access;
          // uint8_t access_type            = *ns->code++;
          // out_field->access_field.type   = access_type & 0x0F;
          // out_field->access_field.mod    = (access_type & 0xC0) >> 6;
          // out_field->access_field.attrib = *ns->code++;
          ns->code += 2;
          return 2;
          break;
        }
      case Connect:
        {
          AML_EXIT();
          parse_namestring(ns);
          // out_field->type               = Connect;
          // out_field->connect_field.name = name;
          break;
        }
      case ExtendedAccess:
        {
          // out_field->type                  = ExtendedAccess;
          // out_field->ext_access_field.type = *ns->code++ & 0x0F;
          // out_field->ext_access_field.mod  = *ns->code++;
          // out_field->ext_access_field.len  = *ns->code++;
          ns->code += 3;
          return 3;
          break;
        }
      default:
        {
          ns->code--;
          hash_key name;
          memcpy(name, ns->code, KEY_LEN);
          ns->code         += KEY_LEN;
          aml_name_t string = (aml_name_t){ KEY_LEN, name };
          if (!valid_name(&string))
            {
              ns->code = code_copy;
              return UINT64_MAX;
            }
          aml_named_field_t* field = malloc(sizeof(aml_named_field_t));
          field->len               = parse_length(ns);
          field->offset            = offset;
          memcpy(field->name, name, KEY_LEN);
          field->parent = parent;
          add_child_to_namespace(
            ns,
            field->name,
            create_ptr(field, TYPE_FIELD_OFFSET)
          );
          return (ns->code - code_copy);
          break;
        }
    }
  return UINT64_MAX;
}

static void populate_package(aml_namespace_t* ns, aml_package_t* pkg)
{
  for (int i = 0; i < pkg->num_elements; i++)
    {
      uint8_t* code_copy = ns->code;
#ifdef DEBUG
      debug_code(ns, 4);
#endif
      aml_name_t*     name     = parse_namestring(ns);
      aml_variable_t* variable = malloc(sizeof(aml_variable_t));
      if (!valid_name(name) || name->count == 0)
        {
          ns->code = code_copy;
          parse_data_object(ns, variable);
        }
      else
        {
          variable->data_type      = DATA_UNINIT;
          aml_name_t resolved_name = resolve_name(ns, *name);
          variable->string         = resolved_name.inner;
        }
      free(name);
      pkg->elements[i] = create_ptr(variable, TYPE_NAME);
    }
#ifdef DEBUG
  aml_log("end\n");
#endif
}

void parse_data_object(aml_namespace_t* ns, aml_variable_t* var)
{
#ifdef DEBUG
  debug_code(ns, 4);
#endif
  uint8_t op = *ns->code++;
  switch (op)
    {
      case ZERO_OP:
        {
          var->data_type = DATA_BYTE;
          var->byte      = 0;
          break;
        }
      case ONE_OP:
        {
          var->data_type = DATA_BYTE;
          var->byte      = 1;
          break;
        }
      case ONES_OP:
        {
          var->data_type = DATA_BYTE;
          var->byte      = 0xFF;
          break;
        }
      case BYTE_PREFIX:
        {
          var->data_type = DATA_BYTE;
          var->byte      = *ns->code++;
          break;
        }
      case WORD_PREFIX:
        {
          uint8_t  lower = *ns->code++;
          uint8_t  upper = *ns->code++;
          uint16_t word  = (uint16_t)upper << 8 | lower;
          var->data_type = DATA_SHORT;
          var->short_int = word;
          break;
        }
      case DWORD_PREFIX:
        {
          uint32_t dword = ns->code[0] | (uint32_t)ns->code[1] << 8 |
                           (uint32_t)ns->code[2] << 16 |
                           (uint32_t)ns->code[3] << 24;
          ns->code      += 4;
          var->data_type = DATA_INT;
          var->integer   = dword;
          break;
        }
      case QWORD_PREFIX:
        {
          uint64_t qword =
            ns->code[0] | (uint64_t)ns->code[1] << 8 |
            (uint64_t)ns->code[2] << 16 | (uint64_t)ns->code[3] << 24 |
            (uint64_t)ns->code[4] << 32 | (uint64_t)ns->code[5] << 40 |
            (uint64_t)ns->code[6] << 48 | (uint64_t)ns->code[7] << 56;
          ns->code      += 8;
          var->data_type = DATA_LONG;
          var->long_int  = qword;
          break;
        }
      case STRING_PREFIX:
        {
          int i = 0;
          while (ns->code[i] != 0) { i++; }
          i++;
          char* ptr = malloc(i * sizeof(char));
          for (int j = 0; j < i; j++) { ptr[j] = *ns->code++; }
          var->data_type = DATA_STR;
          var->string    = ptr;
          break;
        }
      case VAR_PACKAGE_OP:
      case PACKAGE_OP:
        {
          uint8_t* code_copy = ns->code;
          size_t   pkg_len   = parse_length(ns);
          uint8_t  num_elements;
          if (op == PACKAGE_OP) { num_elements = *ns->code++; }
          else
            {
              num_elements = term_arg_to_int(ns);
            }
          aml_package_t* ptr = malloc(sizeof(aml_package_t));
          ptr->elements      = malloc(num_elements * sizeof(aml_variable_t*));
          ptr->num_elements  = num_elements;
          populate_package(ns, ptr);
          var->data_type = DATA_PKG;
          var->package   = ptr;
          break;
        }
      case BUFFER_OP:
        {
          size_t   buf_len     = parse_length(ns);
          uint64_t buffer_size = term_arg_to_int(ns);
          uint8_t* buffer      = malloc(buffer_size * sizeof(uint8_t));
          memcpy(buffer, ns->code, buffer_size);
          aml_buffer_t* ptr = malloc(sizeof(aml_buffer_t));
          ptr->size         = buffer_size;
          ptr->buffer       = buffer;
          ns->code         += buffer_size;
          var->data_type    = DATA_BUF;
          var->buffer       = ptr;
          break;
        }
      default:
        {
          unimplemented(true);
          break;
        }
    }
}

int __aml_log(struct source_location location, char* fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  printf("[%s:%d] ", location.function, location.line);
  int written = vprintf(fmt, args);
  va_end(args);
  return written;
}

bool valid_name(aml_name_t* name)
{
#ifdef DEBUG
  aml_log("\n");
#endif
  int starting_point = 0;
  if (name->inner[0] == '\\' || name->inner[0] == '^') { starting_point = 1; }
  for (int i = starting_point; i < name->count; i++)
    {
#ifdef DEBUG
      printf("%x ", name->inner[i]);
#endif
      if (
        (name->inner[i] < 'A' || name->inner[i] > 'Z') &&
        (name->inner[i] < '0' || name->inner[i] > '9') && name->inner[i] != '_'
      )
        {
#ifdef DEBUG
          putchar('\n');
#endif
          return false;
        }
    }
#ifdef DEBUG
  putchar('\n');
#endif
  return true;
}

