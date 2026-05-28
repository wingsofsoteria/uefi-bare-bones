#include "helpers.h"

#include "defs.h"
#include "name.h"
#include "namespace.h"
#include "opcodes.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

aml_ptr_t* create_ptr(void* data, uint8_t type)
{
  aml_ptr_t* ptr = malloc(sizeof(aml_ptr_t));
  ptr->data      = data;
  ptr->type      = type;
  return ptr;
}

aml_ptr_t* locate_object(aml_namespace_t* ns, aml_name_t key)
{
  alog("%s %s %d\n", ns->name, key.inner, key.count);
  unimplemented(key.count != KEY_LEN);
  unimplemented(key.inner[0] == '\\');
  aml_namespace_t* current = ns;
  aml_ptr_t*       obj     = NULL;
  while (obj == NULL)
    {
      obj = hash_map_get(current->children, key.inner, NULL);
      if (obj) { return obj; }
      current = current->parent;
      if (!current) { break; }
    }
  return NULL;
}

void debug_exit()
{
  debug_namespace(root());
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
      default:
        {
          ns->code--;
          hash_key name;
          memcpy(name, ns->code, KEY_LEN);
          aml_ptr_t* obj =
            locate_object(ns, (aml_name_t){ .count = KEY_LEN, .inner = name });
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
      case (EXT_OP_PREFIX << 8) | 0x83:
        {
          uint8_t* copy = ns->code;
          size_t   len  = parse_length(ns);
          ns->code      = copy + len;
          break;
        }
      default:
        {
          printf("%x\n", op);
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

// current is the namespace that this function was executed in, not necessarily
// the namespace that the key points to key is a valid aml namestring (XXXX,
// ^XYXY, ^YYYY.XXXX, \\XXXX.YYYY.ZZZZ, etc)
aml_namespace_t* get_scope(aml_namespace_t* current, aml_name_t key)
{
  alog("%s %.*s %d\n", current->name, key.count, key.inner, key.count);
  aml_namespace_t* ns        = NULL;
  aml_namespace_t* parent_ns = current;
  size_t           count     = key.count;
  size_t           offset    = 0;
  if (key.inner[0] == '\\')
    {
      if (key.count == 1) return root();
      hash_key parent;
      memcpy(parent, key.inner + 1, KEY_LEN);
      count    -= KEY_LEN + 1;
      offset    = KEY_LEN + 1;
      parent_ns = hash_map_get(root()->namespaces, parent, NULL);
      if (key.count == 5) return parent_ns;
    }
  if (parent_ns)
    {
      alog("%.*s %.*s\n", KEY_LEN, parent_ns->name, count, key.inner + offset);
    }
  unimplemented(key.inner[0] == '^');
  while (count > KEY_LEN)
    {
      hash_key ns_name;
      memcpy(ns_name, key.inner + offset, KEY_LEN);
      offset += KEY_LEN;
      count  -= KEY_LEN;
      if (offset > key.count) debug_exit();
      alog(
        "%.*s %.*s %d %d %.*s\n",
        KEY_LEN,
        ns_name,
        KEY_LEN,
        parent_ns->name,
        count,
        offset,
        count,
        key.inner + offset
      );
      parent_ns = hash_map_get(parent_ns->namespaces, ns_name, NULL);
      if (!parent_ns) { debug_exit(); }
    }
  return hash_map_get(parent_ns->namespaces, key.inner + offset, NULL);
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
          ns->code += KEY_LEN;
          for (int i = 0; i < KEY_LEN; i++)
            {
              if (name[i] >= '0' && name[i] <= '9') { continue; }
              if (name[i] >= 'A' && name[i] <= 'Z') { continue; }
              if (name[i] >= 'a' && name[i] <= 'z') { continue; }
              if (name[i] == '_') { continue; }
              ns->code = code_copy;
              return 0;
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
  return 0;
}

void parse_data_object(aml_namespace_t* ns, void* out_data, uint8_t* out_type)
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
          debug_exit();
          break;
        }
    }
}
