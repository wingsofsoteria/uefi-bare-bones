#include "aml.h"
#include "parser.h"
#include "stdlib.h"

static aml_ptr_t parse_def_alias()
{
  aml_ptr_t source = parse_name_string();
  AML_ERR_CHECK_ABRT(source);
  aml_ptr_t alias = parse_name_string();
  AML_ERR_CHECK_ABRT(alias);
  return (aml_ptr_t){ ALIAS_OP, NULL };
}

aml_ptr_t parse_data_ref_object() { return parse_data_object(); }

static aml_ptr_t parse_def_name(void* map)
{
  aml_ptr_t name = parse_name_string();
  AML_ERR_CHECK_ABRT(name);
  aml_ptr_t object = parse_data_ref_object();
  AML_ERR_CHECK_ABRT(object);
  append(map, last_segment(name.__ptr), object.__ptr);
  return (aml_ptr_t){ NAME_OP, NULL };
}

/*
 * PkgLength is inclusive so ANY calls to parse_pkg_length should also include
 * calls to get_pointer to offset the length correctly
 *
 * PkgLength is encoded as a PkgLeadByte and up to 3 instances of ByteData
 * if PkgLength is greater than 63 then you use the PkgLeadByte to get the
 * number of following ByteData instances each instance of ByteData gets
 * prepended onto the final 32 bit PkgLength
 *
 * this means overwriting the top 4 bits of the PkgLeadByte so the offsets of
 * each ByteData should look like: 4 12 20
 */
uint32_t parse_pkg_length()
{
  char     pkg_lead_byte   = next_byte();
  char     byte_data_count = pkg_lead_byte >> 6;
  uint32_t pkg_length      = (pkg_lead_byte & 0x3F);

  for (int i = 0; i < byte_data_count; i++)
    {
      uint32_t byte = next_byte();
      // I super misread the AML spec for PkgLength encoding
      pkg_length |= (byte << ((i * 8) + 4));
    }
  return pkg_length;
}

static aml_ptr_t parse_def_scope(void* parent_map)
{
  int       current_pointer = get_pointer();
  uint32_t  length          = parse_pkg_length();
  aml_ptr_t scope_name      = parse_name_string();
  int       new_pointer     = get_pointer();
  length                   -= (new_pointer - current_pointer);
  AML_ERR_CHECK_ABRT(scope_name);
  char* key = last_segment(scope_name.__ptr);

  void*     scope_map = new_map(key, 5);
  aml_ptr_t term_list = parse_term_list(scope_map, length);
  AML_ERR_CHECK_ABRT(term_list);
  append(parent_map, key, scope_map);
  return (aml_ptr_t){ SCOPE_OP, NULL };
}

aml_ptr_t parse_namespace_modifier_obj(void* ptr)
{
  uint8_t token = next_byte();
  switch (token)
    {
      case ALIAS_OP: return parse_def_alias();
      case NAME_OP: return parse_def_name(ptr);
      case SCOPE_OP: return parse_def_scope(ptr);
      default: return AML_PREFIX_ERROR;
    }
}
