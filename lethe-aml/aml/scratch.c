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

static void push_namespace(aml_namespace_t* parent, aml_namespace_t* this)
{
  hash_map_push(parent->namespaces, this->name, this, sizeof(aml_namespace_t));
}

static aml_namespace_t* create_namespace(
  aml_namespace_t* parent,
  char*            name,
  uint8_t*         code,
  uint32_t         children,
  uint32_t         namespaces
)
{
  alog("\n");
  aml_namespace_t* namespace = malloc(sizeof(aml_namespace_t));
  assert(namespace != NULL);
  namespace->parent = parent;
  namespace->code   = code;
  size_t name_len   = strlen(name);
  namespace->name   = malloc((name_len + 1) * sizeof(aml_namespace_t));
  memcpy(namespace->name, name, name_len);
  namespace->name[name_len] = 0;
  namespace->children       = hash_map_create(children ? children : 1);
  assert(namespace->children != NULL);
  namespace->namespaces = hash_map_create(namespaces ? namespaces : 1);
  assert(namespace->namespaces != NULL);
  if (parent) { push_namespace(parent, namespace); }
  return namespace;
}

static void prepend_str(aml_name_t* name, char* str, int len)
{
  alog("\n");
  int offset = len;
  assert((len + name->count) < MAX_CHARS);
  if (name->count > 0) { memmove(name->inner + len, name->inner, name->count); }
  memcpy(name->inner, str, len);
  name->count += len;
}

static aml_name_t resolve_name(aml_namespace_t* ns, aml_name_t key)
{
  alog("%s %d\n", key.inner, key.count);
  unimplemented(key.inner[0] == '^' || key.inner[0] == '\\');
  aml_name_t resolved_key = { 0, {} };
  if (key.count > 4 && key.count % 4 == 0)
    {
      prepend_str(&resolved_key, key.inner, key.count);
      prepend_str(&resolved_key, "\\", 1);
      printf("%s -> %s\n", key.inner, resolved_key.inner);
      return resolved_key;
    }
  prepend_str(&resolved_key, key.inner, key.count);
  prepend_str(&resolved_key, ".", 1);
  prepend_str(&resolved_key, ns->name, strlen(ns->name));
  return resolved_key;
}

static aml_ptr_t* create_ptr(void* data, uint8_t type)
{
  alog("\n");
  aml_ptr_t* ptr = malloc(sizeof(aml_ptr_t));
  ptr->data      = data;
  ptr->type      = type;
  return ptr;
}

static aml_namespace_t* locate_namespace(
  aml_namespace_t* parent,
  aml_name_t       key
)
{
  aml_namespace_t* current = parent;
  aml_namespace_t* ns      = NULL;
  while (ns == NULL)
    {
      ns = hash_map_get(current->namespaces, key.inner, NULL);
      if (ns) { return ns; }
      current = current->parent;
      if (!current) { break; }
    }
  return NULL;
}

static aml_ptr_t* locate_object(aml_namespace_t* ns, aml_name_t key)
{
  int              index   = 0;
  aml_namespace_t* current = ns;
  aml_ptr_t*       ptr     = NULL;
  while (ptr == NULL)
    {
      index = -1;
      ptr   = hash_map_get(current->children, key.inner, &index);
      if (ptr != NULL && index != -1) { return ptr; }
      current = current->parent;
      if (current == NULL) { break; }
    }
  return hash_map_get(_root->children, key.inner, NULL);
}

/*
static void* get_child(
  aml_namespace_t* ns,
  aml_name_t       key,
  uint8_t          type,
  uint8_t*         out_type
)
{
  alog("\n");
  int        index = 0;
  aml_ptr_t* ptr   = hash_map_get(ns->children, key.inner, &index);
  if (index == -1 || ptr == NULL) return NULL;
  if (!(ptr->type & type)) return NULL;
  if (out_type != NULL) { *out_type = ptr->type; }
  return ptr->data;
}*/

static void add_child_to_namespace(
  aml_namespace_t* ns,
  aml_name_t       key,
  aml_ptr_t*       data
)
{
  aml_namespace_t* data_parent = ns;
  alog("\n");
  // aml_name_t resolved_name = resolve_name(ns, key);
  if (key.inner[0] == '^')
    {
      data_parent = ns->parent;
      for (int i = 0; i < key.count; i++) { key.inner[i] = key.inner[i + 1]; }
      key.inner[key.count] = 0;
    }
  else if (key.inner[0] == '\\')
    {
      char ns_name[4];
      memcpy(ns_name, key.inner + 1, 4);
      AML_EXIT();
    }
  hash_map_push(data_parent->children, key.inner, data, sizeof(aml_ptr_t));
}

static void debug_namespace(void* ptr)
{
  if (!ptr) { return; }
  aml_namespace_t* ns = ptr;
  printf("%s\n", ns->name);
  hash_map_debug(ns->children);
  hash_map_foreach(ns->namespaces, debug_namespace);
}

static void debug_exit(aml_namespace_t* ns)
{
  alog("\n");
  debug_code(ns, 10);
  printf("\n");
  debug_namespace(_root);
  AML_EXIT();
}

#ifdef __is_libk
static uint64_t read_mem(void* address, uint8_t access_len)
{
  alog("\n");
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
  alog("\n");
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
  alog("\n");
  aml_namespace_t* root_ns = create_namespace(NULL, "\\___", root_code, 8, 4);
  aml_namespace_t* gpe_ns  = create_namespace(root_ns, "_GPE", NULL, 0, 0);
  aml_namespace_t* pr_ns   = create_namespace(root_ns, "_PR_", NULL, 0, 0);
  aml_namespace_t* sb_ns   = create_namespace(root_ns, "_SB_", NULL, 8, 8);
  aml_namespace_t* si_ns   = create_namespace(root_ns, "_SI_", NULL, 0, 0);
  aml_namespace_t* tz_ns   = create_namespace(root_ns, "_TZ_", NULL, 0, 0);
  _root                    = root_ns;
  return 0;
}

static size_t parse_length(aml_namespace_t* ns)
{
  alog("\n");
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
  alog("\n");
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
          memset(aml_name.inner + aml_name.count, 0, 1022 - aml_name.count);
          aml_ptr_t* obj  = locate_object(ns, aml_name);
          void*      ptr  = obj->data;
          uint8_t    type = obj->type;

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
  alog("\n");
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
  alog("\n");
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
  for (int i = 0; i < char_count; i++)
    {
      if (i != 0 && i % 4 == 0)
        {
          printf("%d ", i);
          *name_ptr++ = '.';
          name->count++;
        }
      *name_ptr++ = *copy++;
    }
  *name_ptr = '\0';
  name->count++;
  ns->code = copy;
}

static void def_method(aml_namespace_t* ns)
{
  alog("\n");
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
  alog("\n");
  ns->code = code_copy + method_len;
}

static void parse_data_object(
  aml_namespace_t* ns,
  void*            out_data,
  uint8_t*         out_type
)
{
  alog("\n");
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
      case QWORD_PREFIX:
        {
          *(uint64_t*)out_data =
            ns->code[0] | (uint64_t)ns->code[1] << 8 |
            (uint64_t)ns->code[2] << 16 | (uint64_t)ns->code[3] << 24 |
            (uint64_t)ns->code[4] << 32 | (uint64_t)ns->code[5] << 40 |
            (uint64_t)ns->code[6] << 48 | (uint64_t)ns->code[7] << 56;
          ns->code += 8;
          *out_type = DATA_LONG;
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
  alog("\n");
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
  alog("\n");
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
  alog("\n");
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
  alog("\n");
  uint8_t*   code_copy = ns->code;
  size_t     field_len = parse_length(ns);
  aml_name_t name;
  parse_namestring(ns, &name);
  uint8_t      flags = *ns->code++;
  aml_field_t* field = malloc(sizeof(aml_field_t));
  field->access_type = flags & 0xF;
  field->should_lock = (flags & 0x10) != 0;
  field->update_rule = (flags & 0xC0) >> 6;
  aml_ptr_t* obj     = locate_object(ns, name);
  if (obj->type != TYPE_REGION) { debug_exit(ns); }
  if (obj == NULL) { debug_exit(ns); }
  field->region = obj->data;
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
  alog("\n");
  uint8_t*   code_copy       = ns->code;
  size_t     index_field_len = parse_length(ns);
  aml_name_t name;
  parse_namestring(ns, &name);
  parse_namestring(ns, &name);
  uint8_t flags = *ns->code++;
  ns->code      = code_copy + index_field_len;
}

// todo fix this
static aml_namespace_t* get_scope(aml_namespace_t** out_ns, aml_name_t key)
{
  aml_namespace_t* ns = NULL;
  if (key.inner[0] == '\\')
    {

      char parent[5];
      memcpy(parent, key.inner + 1, 4);
      parent[4] = 0;

      aml_namespace_t* parent_ns =
        hash_map_get(_root->namespaces, parent, NULL);
      if (key.count > 5)
        {
          char ns_name[key.count - 5];
          memcpy(ns_name, key.inner + 5, key.count - 5);
          if (!parent_ns) { debug_exit(ns); }
          *out_ns = parent_ns;
          return hash_map_get(parent_ns->namespaces, ns_name, NULL);
        }
      return parent_ns;
    }
  unimplemented(key.inner[0] == '^');
  ns = locate_namespace(*out_ns, key);
  return ns;
}

static aml_name_t trim_name(aml_name_t* name)
{
  if (name->count <= 5) { return *name; }
  alog("%s -> ", name->inner);
  memmove(name->inner, name->inner + strlen(name->inner) - 4, 5);
  name->count = 5;
  printf("%s\n", name->inner);
  return *name;
}

static void def_scope(aml_namespace_t* ns)
{
  alog("\n");
  uint8_t* code_copy = ns->code;
  size_t   scope_len = parse_length(ns);
  uint8_t* end       = code_copy + scope_len;

  debug_code(ns, 15);
  aml_name_t scope_name;
  parse_namestring(ns, &scope_name);
  if (!scope_name.count)
    {
      parse_termlist(_root, ns->code, end);
      ns->code = end;
      return;
    }
  aml_namespace_t* parent = ns;
  aml_namespace_t* scope  = get_scope(&parent, scope_name);
  if (scope == NULL)
    {
      scope_name = trim_name(&scope_name);
      scope      = create_namespace(parent, scope_name.inner, ns->code, 0, 0);
    }
  parse_termlist(scope, ns->code, end);
  ns->code = end;
}

static void def_device(aml_namespace_t* ns)
{
  alog("\n");
  uint8_t* code_copy  = ns->code;
  size_t   device_len = parse_length(ns);
  uint8_t* end        = code_copy + device_len;
  debug_code(ns, 20);
  aml_name_t device_name;
  parse_namestring(ns, &device_name);
  aml_namespace_t* parent = ns;
  aml_namespace_t* device = get_scope(&parent, device_name);

  if (!device)
    {
      device_name = trim_name(&device_name);
      device      = create_namespace(ns, device_name.inner, ns->code, 4, 0);
    }
  parse_termlist(device, ns->code, end);
  ns->code = end;
}

static void def_alias(aml_namespace_t* ns)
{
  alog("\n");
  aml_name_t source;
  parse_namestring(ns, &source);
  aml_name_t alias;
  parse_namestring(ns, &alias);
  int        index = -1;
  aml_ptr_t* ref   = hash_map_get(ns->children, source.inner, &index);
  if (index == -1 || ref == NULL) { debug_exit(ns); }
  add_child_to_namespace(ns, alias, ref);
}

static void def_mutex(aml_namespace_t* ns)
{
  aml_name_t mutex_name;
  parse_namestring(ns, &mutex_name);
  aml_mutex_t* mutex = malloc(sizeof(aml_mutex_t));
  mutex->name        = mutex_name;
  mutex->sync_flags  = *ns->code++;
  add_child_to_namespace(ns, mutex_name, create_ptr(mutex, TYPE_MUTEX));
}

static void parse_next(aml_namespace_t* ns)
{
  alog("\n");
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
  alog("\n");
  uint8_t* copy = ns->code;
  ns->code      = start;
  while (ns->code < end) { parse_next(ns); }
  ns->code = copy;
}

static void populate_children(aml_namespace_t* ns, size_t table_len)
{ parse_termlist(ns, ns->code, ns->code + table_len); }

void parse_table(acpi_aml_table_t* table)
{
  alog("\n");
  if (!_root)
    {
      if (init_aml_namespaces(table->definition_blocks)) { return; }
    }
  populate_children(_root, table->length - 36);
}
