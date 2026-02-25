#include "aml.h"
#include "parser.h"
#include "stdlib.h"
#include <stdio.h>

aml_ptr_t parse_def_alias()
{
  uint8_t token = next_byte();
  if (token != ALIAS_OP) return AML_ERROR;
  printf("DefAlias\n");
  aml_ptr_t source = parse_name_string();
  aml_ptr_t alias  = parse_name_string();
  return (aml_ptr_t){ALIAS_OP, NULL};
}

aml_ptr_t parse_def_name()
{
  uint8_t token = next_byte();
  if (token != NAME_OP) return AML_ERROR;
  printf("DefName\n");
  parse_name_string();
  parse_data_ref_object();
  return (aml_ptr_t){NAME_OP, NULL};
}

uint32_t parse_pkg_length()
{
  char pkg_lead_byte    = next_byte();
  char byte_data_count  = pkg_lead_byte >> 6;
  char pkg_length_upper = (pkg_lead_byte >> 4) & 0x03;
  char pkg_length_lower = (pkg_lead_byte & 0x0F);
  uint32_t pkg_length   = (pkg_length_upper << 4) | pkg_length_lower;
  while (byte_data_count > 0)
  {
    char next_lsb = next_byte();
    pkg_length    = (pkg_length << 8) | next_lsb;
    byte_data_count--;
  }
  return pkg_length;
}

aml_ptr_t parse_def_scope()
{
  uint8_t token = next_byte();
  if (token != SCOPE_OP) return AML_ERROR;
  printf("DefScope\n");
  parse_pkg_length();
  parse_name_string();
  parse_term_list();
  return (aml_ptr_t){SCOPE_OP, NULL};
}

aml_ptr_t parse_namespace_modifier_obj()
{
  printf("NamespaceModifierObj\n");
  return one_of(3, parse_def_alias, parse_def_name, parse_def_scope);
}
