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

#define VARIABLE_SIG "VAR_"

typedef struct
{
  char       sig[4];
  aml_name_t label;
  uint8_t    data_type;
  void*      data;
} aml_variable_t;

typedef struct
{
  size_t   size;
  uint8_t* buffer;
} aml_buffer_t;

#define METHOD_SIG "METD"

typedef struct
{
  char       sig[4];
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
  enum FieldUnitType type;

  union
  {
    struct
    {
      aml_name_t name;
      size_t     len;
    } named_field;

    struct
    {
      size_t len;
    } reserved_field;

    struct
    {
      enum AccessType        type;
      enum AttributeModifier mod;
      enum AccessAttrib      attrib;
    } access_field;

    struct
    {
      enum AccessType        type;
      enum ExtAccessModifier mod;
      uint8_t                len;
    } ext_access_field;

    struct
    {
      union
      {
        aml_name_t   name;
        aml_buffer_t buffer;
      };
    } connect_field;
  };
} aml_field_unit_t;

typedef struct
{
  enum AccessType   access_type;
  bool              should_lock;
  enum UpdateRule   update_rule;
  aml_field_unit_t* field_unit_list;
} aml_field_t;

#define OP_REGION_SIG "OPRN"

typedef struct
{
  char         sig[4];
  aml_name_t   name;
  uint8_t      region_space;
  size_t       offset;
  size_t       len;
  aml_field_t* field;
} aml_operation_region_t;

typedef struct
{
  char  sig[4];
  void* __unused;
} __aml_generic_t;

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

static void print_field(aml_field_t* field)
{
  int indent = print_indent(-1);
  printf("Field\n");
  print_indent(-1);
  printf("{\n");
  print_indent(indent + 2);
  switch (field->access_type)
    {
      case AnyAcc:
        {
          printf("AnyAcc\n");
          break;
        }
      case ByteAcc:
        {
          printf("ByteAcc\n");
          break;
        }
      case WordAcc:
        {
          printf("WordAcc\n");
          break;
        }
      case DWordAcc:
        {
          printf("DWordAcc\n");
          break;
        }
      case QWordAcc:
        {
          printf("QWordAcc\n");
          break;
        }
      case BufferAcc:
        {
          printf("QWordAcc\n");
          break;
        }
      default:
        {
          printf("???\n");
          break;
        }
    }
  print_indent(-1);
  printf("%s\n", (int)field->should_lock ? "Lock" : "NoLock");
  print_indent(-1);
  switch (field->update_rule)
    {
      case Preserve:
        {
          printf("Preserve\n");
          break;
        }
      case WriteAsOnes:
        {
          printf("WriteAsOnes\n");
          break;
        }
      case WriteAsZeros:
        {
          printf("WriteAsZeros\n");
          break;
        }
      default:
        {
          printf("???\n");
          break;
        }
    }
  print_indent(-1);
  printf("List\n");
  print_indent(-1);
  printf("{\n");
  aml_field_unit_t unit = field->field_unit_list[0];

  int i = 0;
  while (unit.type != 0xFE)
    {
      unit = field->field_unit_list[i];
      print_indent(indent + 4);
      switch (unit.type)
        {
          case Reserved:
            {
              printf("Reserved: %#lx\n", unit.reserved_field.len);
              break;
            }
          case Access:
            {
              printf(
                "Access: %d %d %d\n",
                unit.access_field.type,
                unit.access_field.attrib,
                unit.access_field.mod
              );
              break;
            }
          case Connect:
            {
              printf("Connect: %s\n", unit.connect_field.name.inner);
              break;
            }
          case ExtendedAccess:
            {
              printf(
                "ExtAccess: %d %d %d\n",
                unit.ext_access_field.type,
                unit.ext_access_field.mod,
                unit.ext_access_field.len
              );
              break;
            }
          case Named:
            {
              printf(
                "Named: %s %lu\n",
                unit.named_field.name.inner,
                unit.named_field.len
              );
              break;
            }
          default:
            {
              printf("???\n");
              break;
            }
        }
      i++;
    }
  print_indent(indent + 2);
  printf("}\n");
  print_indent(indent);
  printf("}\n");
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
  print_field(region->field);
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
  void* data;
  printf("Root\n{\n");
  int indent = 2;
  hash_map_iter(ns->children, data)
  {
    __aml_generic_t* _data = (__aml_generic_t*)data;
    if (!data)
      {
        print_indent(indent);
        printf("Empty\n");
        continue;
      }
    if (strncmp(_data->sig, VARIABLE_SIG, 4) == 0)
      {
        print_variable(data);
        continue;
      }
    if (strncmp(_data->sig, OP_REGION_SIG, 4) == 0)
      {
        print_region(data);
        continue;
      }
    if (strncmp(_data->sig, METHOD_SIG, 4) == 0)
      {
        print_method(data);
        continue;
      }
    print_indent(indent);
    printf("%.4s\n", _data->sig);
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
          char name[4] = { ns->code[0], ns->code[1], ns->code[2], ns->code[3] };
          int  index   = -1;
          aml_variable_t* var = hash_map_get(ns->children, name, &index);

          if (index == -1)
            {
              debug_exit(ns);
              return 1;
            }
          if (strncmp(var->sig, VARIABLE_SIG, 4) != 0)
            {
              debug_exit(ns);
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
  uint8_t       method_flags = *ns->code++;
  aml_method_t* method       = malloc(sizeof(aml_method_t));
  memcpy(method->sig, METHOD_SIG, 4);
  method->flags = method_flags;
  method->len   = method_len - (ns->code - code_copy);
  method->name  = method_name;
  method->code  = ns->code;
  hash_map_push(ns->children, method_name.inner, method, sizeof(aml_method_t));
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
          *(uint64_t*)out_data = 0;
          *out_type            = 1;
          break;
        }
      case ONE_OP:
        {
          *(uint64_t*)out_data = 1;
          *out_type            = 1;
          break;
        }
      case ONES_OP:
        {
          *(uint64_t*)out_data = 0xFF;
          *out_type            = 1;
          break;
        }
      case BYTE_PREFIX:
        {
          uint64_t data        = *ns->code++;
          *(uint64_t*)out_data = data;
          *out_type            = 1;
          break;
        }
      case WORD_PREFIX:
        {
          uint8_t  lower       = *ns->code++;
          uint8_t  upper       = *ns->code++;
          uint16_t word        = (uint16_t)upper << 8 | lower;
          *(uint64_t*)out_data = word;
          *out_type            = 1;
          break;
        }
      case DWORD_PREFIX:
        {
          *(uint64_t*)out_data = ns->code[0] | (uint32_t)ns->code[1] << 8 |
                                 (uint32_t)ns->code[2] << 16 |
                                 (uint32_t)ns->code[3] << 24;
          ns->code            += 4;
          *out_type            = 1;
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
          *out_type = 2;
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
  memcpy(name_var->sig, VARIABLE_SIG, 4);
  name_var->data      = data;
  name_var->label     = name;
  name_var->data_type = type;
  hash_map_push(ns->children, name.inner, name_var, sizeof(name_var));
  hash_map_debug(ns->children);
}

static void def_op_region(aml_namespace_t* ns)
{
  aml_name_t name;
  parse_namestring(ns, &name);
  uint8_t                 region_space  = *ns->code++;
  uint64_t                region_offset = term_arg_to_int(ns);
  uint64_t                region_len    = term_arg_to_int(ns);
  aml_operation_region_t* region = malloc(sizeof(aml_operation_region_t));
  memcpy(region->sig, OP_REGION_SIG, 4);
  region->region_space = region_space;
  region->len          = region_len;
  region->offset       = region_offset;
  region->name         = name;
  region->field        = NULL;
  hash_map_push(
    ns->children,
    name.inner,
    region,
    sizeof(aml_operation_region_t)
  );
}

static int parse_next_field_elem(
  aml_namespace_t*  ns,
  aml_field_unit_t* out_field
)
{
  uint8_t* code_copy = ns->code;
  uint8_t  op        = *ns->code++;
  switch (op)
    {
      case Reserved:
        {
          out_field->type               = Reserved;
          out_field->reserved_field.len = parse_length(ns);
          break;
        }
      case Access:
        {
          out_field->type                = Access;
          uint8_t access_type            = *ns->code++;
          out_field->access_field.type   = access_type & 0x0F;
          out_field->access_field.mod    = (access_type & 0xC0) >> 6;
          out_field->access_field.attrib = *ns->code++;
          break;
        }
      case Connect:
        {
          AML_EXIT();
          aml_name_t name;
          parse_namestring(ns, &name);
          out_field->type               = Connect;
          out_field->connect_field.name = name;
          break;
        }
      case ExtendedAccess:
        {
          out_field->type                  = ExtendedAccess;
          out_field->ext_access_field.type = *ns->code++ & 0x0F;
          out_field->ext_access_field.mod  = *ns->code++;
          out_field->ext_access_field.len  = *ns->code++;
          break;
        }
      default:
        {
          aml_name_t name;
          parse_namestring(ns, &name);
          for (int i = 0; i < 4; i++)
            {
              if (name.inner[i] >= '0' && name.inner[i] <= '9') { continue; }
              if (name.inner[i] >= 'A' && name.inner[i] <= 'Z') { continue; }
              if (name.inner[i] >= 'a' && name.inner[i] <= 'z') { continue; }
              ns->code = code_copy;
              return 1;
            }
          out_field->type             = Named;
          out_field->named_field.name = name;
          out_field->named_field.len  = parse_length(ns);
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
  int                     index = -1;
  aml_operation_region_t* region =
    hash_map_get(ns->children, name.inner, &index);
  assert(index != -1);
  assert(strncmp(region->sig, OP_REGION_SIG, 4) == 0);
  uint8_t      flags = *ns->code++;
  aml_field_t* field = malloc(sizeof(aml_field_t));
  field->access_type = flags & 0xF;
  field->should_lock = (flags & 0x10) != 0;
  field->update_rule = (flags & 0xC0) >> 6;
  assert((flags & 0x80) == 0);
  uint8_t* b_code_copy = ns->code;
  size_t   field_count = 1;
  while (1)
    {
      aml_field_unit_t unit;
      if (parse_next_field_elem(ns, &unit)) { break; }
      field_count++;
    }
  ns->code               = b_code_copy;
  field->field_unit_list = malloc(field_count * sizeof(aml_field_unit_t));
  int i                  = 0;
  while (1)
    {
      if (parse_next_field_elem(ns, &field->field_unit_list[i])) { break; }
      i++;
    }
  field->field_unit_list[field_count - 1] = (aml_field_unit_t){ .type = 0xFE };
  region->field                           = field;
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
