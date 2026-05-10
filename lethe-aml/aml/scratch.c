/*
 * method: (name, code) should be stored within a namespace (scope, device, etc)
 * */

#include "aml.h"
#include "hashmap.h"
#include "host.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TYPE_METHOD    (1 << 0)
#define TYPE_NAMESPACE (1 << 1)
#define TYPE_REGION    (1 << 2)
#define TYPE_NAME      (1 << 3)
#define TYPE_FIELD     (1 << 4)

#define DATA_STR   (1 << 0)
#define DATA_PKG   (1 << 1)
#define DATA_BUF   (1 << 2)
#define DATA_BYTE  (1 << 3)
#define DATA_SHORT (1 << 4)
#define DATA_INT   (1 << 5)

typedef struct aml_namespace
{
  struct aml_namespace* parent;
  char*                 name;
  uint8_t*              code;
  hash_map_t*           children;
} aml_namespace_t;

#define MAX_CHARS 1022

typedef struct
{
  int  count;
  char inner[MAX_CHARS];
} aml_name_t;

typedef struct
{
  aml_name_t label;
  uint8_t    data_type;
  void*      data;
} aml_variable_t;

typedef struct
{
  size_t   size;
  uint8_t* buffer;
} aml_buffer_t;

typedef struct
{
  aml_name_t name;
  uint8_t    flags;
  size_t     len;
  uint8_t*   code;
} aml_method_t;

enum VariableType
{
  VariableUnimplemented = 0,
  VariableInt           = 1,
  VariableStr           = 2,
};

enum AccessType
{
  AnyAcc    = 0,
  ByteAcc   = 1,
  WordAcc   = 2,
  DWordAcc  = 3,
  QWordAcc  = 4,
  BufferAcc = 5
};

enum UpdateRule
{
  Preserve     = 0,
  WriteAsOnes  = 1,
  WriteAsZeros = 2
};

enum FieldUnitType
{
  Reserved       = 0x00,
  Access         = 0x01,
  Connect        = 0x02,
  ExtendedAccess = 0x03,
  Named          = 0xFF,
};

enum AttributeModifier
{
  AttribNormal          = 0,
  AttribBytes           = 1,
  AttribRawBytes        = 2,
  AttribRawProcessBytes = 3,
};

enum AccessAttrib
{
  AttribQuick            = 0x02,
  AttribSendRecv         = 0x04,
  AttribByte             = 0x06,
  AttribWord             = 0x08,
  AttribBlock            = 0x0A,
  AttribProcessCall      = 0x0C,
  AttribBlockProcessCall = 0x0D
};

enum ExtAccessModifier
{
  ExtAttribBytes      = 0x0B,
  ExtAttribRawBytes   = 0x0E,
  ExtAttribRawProcess = 0x0F
};

typedef struct
{
  enum AccessType access_type;
  bool            should_lock;
  enum UpdateRule update_rule;
  aml_name_t      name;
} aml_field_t;

typedef struct
{
  int          offset;
  int          len;
  aml_name_t   name;
  aml_field_t* parent;
} aml_named_field_t;

typedef struct
{
  aml_name_t name;
  uint8_t    region_space;
  size_t     offset;
  size_t     len;
} aml_operation_region_t;

typedef struct
{
  uint8_t type;
  void*   data;
} aml_ptr_t;

static aml_namespace_t* _root = NULL;

static aml_namespace_t* create_namespace(
  aml_namespace_t* parent,
  char*            name,
  uint8_t*         code
)
{
  aml_namespace_t* namespace = malloc(sizeof(aml_namespace_t));
  namespace->parent          = parent;
  namespace->code            = code;
  namespace->name            = name;
  hash_map_t* map;
  hash_map_create(&map, 5);
  namespace->children = map;
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

static void* search_root(aml_name_t key)
{
  if (key.inner[0] == '\\')
    {
      int index = 0;
      return hash_map_get(_root->children, key.inner, &index);
    }
  return NULL;
}

static aml_ptr_t* create_ptr(void* data, uint8_t type)
{
  aml_ptr_t* ptr = malloc(sizeof(aml_ptr_t));
  ptr->data      = data;
  ptr->type      = type;
  return ptr;
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

static int print_indent(int indent)
{
  static int static_indent = 0;
  if (indent > 0) { static_indent = indent; };
  printf("%*s", static_indent, " ");
  return static_indent;
}

static void print_variable(aml_variable_t* var)
{
  int prev_indent = 0;
  int indent      = print_indent(-1);
  prev_indent     = indent;
  printf("Name\n");
  print_indent(-1);
  printf("{\n");
  print_indent(indent + 2);
  printf("%s\n", var->label.inner);
  print_indent(-1);
  switch (var->data_type)
    {
      case VariableInt:
        {
          printf("%#lx\n", *(uint64_t*)var->data);
          break;
        };
      case VariableStr:
        {
          printf("%s\n", (char*)var->data);
          break;
        };
      case VariableUnimplemented:
      default:
        {
          printf("%p\n", var->data);
          break;
        }
    };
  print_indent(indent);
  printf("}");
  print_indent(prev_indent);
  printf("\n");
}

static char* stringify_region_space(uint8_t space)
{
  char* value = "???";
  switch (space)
    {
      case 0: value = "SystemMemory"; break;
      case 1: value = "SystemIO"; break;
      case 2: value = "PCI_Config"; break;
      case 3: value = "EmbeddedControl"; break;
      case 4: value = "SMBus"; break;
      case 5: value = "System CMOS"; break;
      case 6: value = "PciBarTarget"; break;
      case 7: value = "IPMI"; break;
      case 8: value = "GeneralPurposeIO"; break;
      case 9: value = "GenericSerialBus"; break;
      case 10: value = "PCC"; break;
      default: value = "OEM Defined"; break;
    }
  return value;
}

static void print_region(aml_operation_region_t* region)
{
  int prev_indent = 0;
  prev_indent     = print_indent(-1);
  printf("OperationRegion\n");
  print_indent(-1);
  printf("{\n");
  print_indent(prev_indent + 2);
  printf("%s\n", region->name.inner);
  print_indent(-1);
  printf(
    "%s(%#x)\n",
    stringify_region_space(region->region_space),
    region->region_space
  );
  print_indent(-1);
  printf("%#lx\n", region->offset);
  print_indent(-1);
  printf("%#lx\n", region->len);
  print_indent(prev_indent);
  printf("}\n");
}

static void print_method(aml_method_t* method)
{
  int indent = print_indent(-1);
  printf("Method\n");
  print_indent(-1);
  printf("{\n");
  print_indent(indent + 2);
  printf("%s\n", method->name.inner);
  print_indent(-1);
  printf("%d\n", method->flags);
  print_indent(-1);
  printf("%lu\n", method->len);
  print_indent(-1);
  printf("%p\n", method->code);
  print_indent(indent);
  printf("}\n");
}

static void print_children(aml_namespace_t* ns)
{
  aml_ptr_t* data;
  printf("%s\n{\n", ns->name);
  int indent = -1;
  if (ns->parent == NULL) { indent = 2; };
  hash_map_iter(ns->children, data)
  {
    switch (data->type)
      {
        case TYPE_NAME:
          {
            print_variable(data->data);
            break;
          }
        case TYPE_REGION:
          {
            print_region(data->data);
            break;
          }
        case TYPE_METHOD:
          {
            print_method(data->data);
            break;
          }
        default:
          {
            print_indent(indent);
            printf("Empty\n");
            break;
          }
      }
  }
  printf("}\n");
  fflush(stdout);
}

static void debug_exit(aml_namespace_t* ns)
{
  print_children(ns);
  AML_EXIT();
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
          int        index = -1;
          aml_ptr_t* var   = hash_map_get(
            ns->children,
            resolve_name(_root, aml_name).inner,
            &index
          );

          if (index == -1)
            {
              debug_exit(ns);
              return 1;
            }
          switch (var->type)
            {
              case TYPE_NAME:
                {
                  ns->code            += 4;
                  aml_variable_t* data = var->data;
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
              default:
                {
                  debug_exit(ns);
                  return 1;
                }
            }
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
  aml_name_t parent_name = resolve_name(ns, parent->name);
  uint8_t*   code_copy   = ns->code;
  uint8_t    op          = *ns->code++;
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
          prepend_str(&name, parent_name.inner, parent_name.count);
          field->name   = name;
          field->parent = parent;
          add_child_to_namespace(
            ns,
            field->name,
            create_ptr(field, TYPE_FIELD)
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
  field->name        = name;
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
          debug_exit(ns);
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
