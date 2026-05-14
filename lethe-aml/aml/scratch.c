#include "aml.h"
#include "hashmap.h"
#include "host.h"

#include <assert.h>
#include <stdalign.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static aml_namespace_t* _root = NULL;

static aml_namespace_t* create_namespace(
  aml_namespace_t* parent,
  char*            name,
  uint8_t*         code
)
{
  aml_namespace_t* namespace = malloc(sizeof(aml_namespace_t));
  assert(namespace != NULL);
  namespace->parent = parent;
  namespace->code   = code;
  size_t name_len   = strlen(name);
  namespace->name   = malloc((name_len + 1) * sizeof(aml_namespace_t));
  memcpy(namespace->name, name, name_len);
  namespace->name[name_len] = 0;
  namespace->children       = hash_map_create(5);
  assert(namespace->children != NULL);
  return namespace;
}

static void prepend_str(aml_name_t* name, char* str, int len)
{
  int offset = len;
  assert((len + name->count) < MAX_CHARS);
  if (name->count > 0) { memmove(name->inner + len, name->inner, name->count); }
  memcpy(name->inner, str, len);
  name->count += len;
}

static aml_name_t resolve_name(aml_namespace_t* ns, aml_name_t key)
{
  if (key.inner[0] == '\\') { return key; }
  unimplemented(key.inner[0] == '^');
  unimplemented(key.count != 4);
  aml_name_t resolved_key = { 0, {} };
  prepend_str(&resolved_key, key.inner, key.count);
  if (ns->parent != NULL)
    {
      prepend_str(&resolved_key, ".", 1);
      prepend_str(&resolved_key, ns->name, strlen(ns->name));
      return resolved_key;
    }
  prepend_str(&resolved_key, "\\", 1);
  return resolved_key;
}

static aml_ptr_t* create_ptr(void* data, uint8_t type)
{
  aml_ptr_t* ptr = malloc(sizeof(aml_ptr_t));
  ptr->data      = data;
  ptr->type      = type;
  return ptr;
}

static void* get_child(
  aml_namespace_t* ns,
  aml_name_t       key,
  uint8_t          type,
  uint8_t*         out_type
)
{
  int        index    = 0;
  aml_name_t resolved = resolve_name(ns, key);
  aml_ptr_t* ptr      = hash_map_get(ns->children, resolved.inner, &index);
  if (index == -1) return NULL;
  if (!(ptr->type & type)) return NULL;
  if (out_type != NULL) { *out_type = ptr->type; }
  return ptr->data;
}

static void add_child_to_namespace(
  aml_namespace_t* ns,
  aml_name_t       key,
  aml_ptr_t*       data
)
{
  aml_name_t resolved_name = resolve_name(ns, key);
  hash_map_push(ns->children, resolved_name.inner, data, sizeof(aml_ptr_t));
}

static void debug_exit(aml_namespace_t* ns)
{
  printf("%s\n", ns->name);
  hash_map_debug(ns->children);
  AML_EXIT();
}

#ifdef __is_libk
static uint64_t read_mem(void* address, uint8_t access_len)
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
          debug_exit(_root);
        }
    }
  return 0;
}
#else
static uint64_t read_mem(void* address, uint8_t access_len) { return 0; }
#endif

static uint64_t read_io(uint16_t port, uint8_t access_len)
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
          debug_exit(_root);
        }
    }
  return 0;
}

static int init_aml_namespaces(uint8_t* root_code)
{
  aml_namespace_t* root_ns = create_namespace(NULL, "\\___", root_code);
  aml_namespace_t* gpe_ns  = create_namespace(NULL, "\\_GPE", NULL);
  aml_namespace_t* pr_ns   = create_namespace(NULL, "\\_PR_", NULL);
  aml_namespace_t* sb_ns   = create_namespace(NULL, "\\_SB_", NULL);
  aml_namespace_t* si_ns   = create_namespace(NULL, "\\_SI_", NULL);
  aml_namespace_t* tz_ns   = create_namespace(NULL, "\\_TZ_", NULL);
  hash_map_push(root_ns->children, "\\_GPE", gpe_ns, sizeof(aml_namespace_t));
  hash_map_push(root_ns->children, "\\_PR_", pr_ns, sizeof(aml_namespace_t));
  hash_map_push(root_ns->children, "\\_SB_", sb_ns, sizeof(aml_namespace_t));
  hash_map_push(root_ns->children, "\\_SI_", si_ns, sizeof(aml_namespace_t));
  hash_map_push(root_ns->children, "\\_TZ_", tz_ns, sizeof(aml_namespace_t));
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
          aml_name_t aml_name;
          aml_name.count = 4;
          memcpy(aml_name.inner, ns->code, 4);
          uint8_t type = 0;
          void*   ptr =
            get_child(ns, aml_name, TYPE_NAME | TYPE_FIELD_OFFSET, &type);
          if (ptr == NULL)
            {
              debug_exit(ns);
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
                      return *(uint8_t*)data->data;
                    }
                  case DATA_SHORT:
                    {
                      return *(uint16_t*)data->data;
                    }
                  case DATA_INT:
                    {
                      return *(uint32_t*)data->data;
                    }
                  default:
                    {
                      debug_exit(ns);
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
                      debug_exit(ns);
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
                      debug_exit(ns);
                    }
                }
              return read_io(region->offset + data->offset, read_len);
            }
          return 0;
        }
    }
}

static void parse_termlist(
  aml_namespace_t* ns,
  uint8_t*         start,
  const uint8_t*   end
);

static void def_if_else(aml_namespace_t* ns)
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
  name->count         = char_count;
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
  uint8_t       method_flags = *ns->code++;
  aml_method_t* method       = malloc(sizeof(aml_method_t));
  method->flags              = method_flags;
  method->len                = method_len - (ns->code - code_copy);
  method->name               = method_name;
  method->code               = ns->code;
  add_child_to_namespace(ns, method_name, create_ptr(method, TYPE_METHOD));
  ns->code = code_copy + method_len;
}

static void parse_data_object(
  aml_namespace_t* ns,
  void*            out_data,
  uint8_t*         out_type
)
{
  uint8_t op = *ns->code++;
  switch (op)
    {
      case ZERO_OP:
        {
          *(uint8_t*)out_data = 0;
          *out_type           = DATA_BYTE;
          break;
        }
      case ONE_OP:
        {
          *(uint8_t*)out_data = 1;
          *out_type           = DATA_BYTE;
          break;
        }
      case ONES_OP:
        {
          *(uint8_t*)out_data = 0xFF;
          *out_type           = DATA_BYTE;
          break;
        }
      case BYTE_PREFIX:
        {
          uint8_t data        = *ns->code++;
          *(uint8_t*)out_data = data;
          *out_type           = DATA_BYTE;
          break;
        }
      case WORD_PREFIX:
        {
          uint8_t  lower       = *ns->code++;
          uint8_t  upper       = *ns->code++;
          uint16_t word        = (uint16_t)upper << 8 | lower;
          *(uint16_t*)out_data = word;
          *out_type            = DATA_SHORT;
          break;
        }
      case DWORD_PREFIX:
        {
          *(uint32_t*)out_data = ns->code[0] | (uint32_t)ns->code[1] << 8 |
                                 (uint32_t)ns->code[2] << 16 |
                                 (uint32_t)ns->code[3] << 24;
          ns->code            += 4;
          *out_type            = DATA_INT;
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
          out_data  = ptr;
          *out_type = DATA_STR;
          break;
        }
      case PACKAGE_OP:
        {
          uint8_t*      code_copy    = ns->code;
          size_t        pkg_len      = parse_length(ns);
          uint8_t       num_elements = *ns->code++;
          aml_buffer_t* ptr          = out_data;
          ptr->buffer                = ns->code;
          ptr->size                  = num_elements;
          ns->code                   = code_copy + pkg_len;
          *out_type                  = DATA_PKG;
          break;
        }
      case BUFFER_OP:
        {
          size_t   buf_len     = parse_length(ns);
          uint64_t buffer_size = term_arg_to_int(ns);
          uint8_t* buffer      = malloc(buffer_size * sizeof(uint8_t));
          memcpy(buffer, ns->code, buffer_size);
          aml_buffer_t* ptr = out_data;
          ptr->size         = buffer_size;
          ptr->buffer       = buffer;
          *out_type         = DATA_BUF;
          ns->code         += buffer_size;
          break;
        }
      default:
        {
          debug_exit(ns);
          break;
        }
    }
}

static void def_name(aml_namespace_t* ns)
{
  aml_name_t name;
  parse_namestring(ns, &name);
  void*   data = malloc(sizeof(void*));
  uint8_t type = 0;
  parse_data_object(ns, data, &type);
  aml_variable_t* name_var = malloc(sizeof(aml_variable_t));
  name_var->data           = data;
  name_var->label          = name;
  name_var->data_type      = type;

  add_child_to_namespace(ns, name, create_ptr(name_var, TYPE_NAME));
}

static void parse_next(aml_namespace_t* /*ns*/);

static void def_op_region(aml_namespace_t* ns)
{
  aml_name_t name;
  parse_namestring(ns, &name);
  uint8_t                 region_space  = *ns->code++;
  uint64_t                region_offset = term_arg_to_int(ns);
  uint64_t                region_len    = term_arg_to_int(ns);
  aml_operation_region_t* region = malloc(sizeof(aml_operation_region_t));
  region->region_space           = region_space;
  region->len                    = region_len;
  region->offset                 = region_offset;
  region->name                   = name;
  add_child_to_namespace(ns, name, create_ptr(region, TYPE_REGION));
}

static size_t parse_next_field_elem(
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
          aml_name_t name;
          parse_namestring(ns, &name);
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
          aml_name_t name;
          parse_namestring(ns, &name);
          for (int i = 0; i < 4; i++)
            {
              if (name.inner[i] >= '0' && name.inner[i] <= '9') { continue; }
              if (name.inner[i] >= 'A' && name.inner[i] <= 'Z') { continue; }
              if (name.inner[i] >= 'a' && name.inner[i] <= 'z') { continue; }
              if (name.inner[i] == '_') { continue; }
              ns->code = code_copy;
              return 0;
            }
          aml_named_field_t* field = malloc(sizeof(aml_named_field_t));
          field->len               = parse_length(ns);
          field->offset            = offset;
          field->name              = name;
          field->parent            = parent;
          add_child_to_namespace(
            ns,
            field->name,
            create_ptr(field, TYPE_FIELD_OFFSET)
          );
          return (ns->code - code_copy);
          break;
        }
    }
  return 0;
}

static void def_field(aml_namespace_t* ns)
{
  uint8_t*   code_copy = ns->code;
  size_t     field_len = parse_length(ns);
  aml_name_t name;
  parse_namestring(ns, &name);
  uint8_t      flags = *ns->code++;
  aml_field_t* field = malloc(sizeof(aml_field_t));
  field->access_type = flags & 0xF;
  field->should_lock = (flags & 0x10) != 0;
  field->update_rule = (flags & 0xC0) >> 6;
  void* region       = get_child(ns, name, TYPE_REGION, NULL);
  if (region == NULL) { debug_exit(ns); }
  field->region = region;
  assert((flags & 0x80) == 0);
  int offset = 0;
  while (1)
    {
      size_t len = parse_next_field_elem(ns, offset, field);
      if (len == 0) { break; }
      offset += len;
    }
  // TODO parse field list
  // append field list to operation region
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

static void def_scope(aml_namespace_t* ns)
{
  uint8_t*   code_copy = ns->code;
  size_t     scope_len = parse_length(ns);
  uint8_t*   end       = code_copy + scope_len;
  aml_name_t scope_name;
  parse_namestring(ns, &scope_name);
  aml_namespace_t* scope = get_child(ns, scope_name, TYPE_NAMESPACE, NULL);
  if (scope == NULL)
    {
      int index = -1;
      scope     = hash_map_get(
        _root->children,
        resolve_name(_root, scope_name).inner,
        &index
      );
      if (index == -1 || scope == NULL)
        {
          scope =
            create_namespace(ns, resolve_name(ns, scope_name).inner, ns->code);
          add_child_to_namespace(
            ns,
            scope_name,
            create_ptr(scope, TYPE_NAMESPACE)
          );
        }
    }

  parse_termlist(scope, ns->code, end);
}

static void def_alias(aml_namespace_t* ns)
{
  aml_name_t source;
  parse_namestring(ns, &source);
  source = resolve_name(ns, source);
  aml_name_t alias;
  parse_namestring(ns, &alias);
  int        index = -1;
  aml_ptr_t* ref   = hash_map_get(ns->children, source.inner, &index);
  if (index == -1 || ref == NULL) { debug_exit(ns); }
  add_child_to_namespace(ns, alias, ref);
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
      case SCOPE_OP:
        {
          def_scope(ns);
          break;
        }
      case ALIAS_OP:
        {
          def_alias(ns);
          break;
        }
      default:
        {
          printf("%x\n", op);
          debug_exit(ns);
          break;
        }
    }
}

static void parse_termlist(
  aml_namespace_t* ns,
  uint8_t*         start,
  const uint8_t*   end
)
{
  uint8_t* copy = ns->code;
  ns->code      = start;
  while (ns->code < end) { parse_next(ns); }
  ns->code = copy;
}

static void populate_children(aml_namespace_t* ns, size_t table_len)
{ parse_termlist(ns, ns->code, ns->code + table_len); }

void parse_table(acpi_aml_table_t* table)
{
  if (!_root)
    {
      if (init_aml_namespaces(table->definition_blocks)) { return; }
    }
  populate_children(_root, table->length - 36);
}
