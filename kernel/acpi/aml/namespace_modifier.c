#include "aml.h"
#include "parser.h"
#include "stdlib.h"
#include <stdio.h>

aml_ptr_t parse_def_alias()
{
  AML_PRELUDE(ALIAS_OP)
  aml_ptr_t source = parse_name_string();
  AML_ERR_CHECK(source);
  aml_ptr_t alias = parse_name_string();
  AML_ERR_CHECK(alias);
  printf("DefAlias ");
  print_name_string(source);
  putchar(' ');
  print_name_string(alias);
  putchar(' ');
  return (aml_ptr_t){ALIAS_OP, NULL};
}

aml_ptr_t parse_data_ref_object()
{
  return parse_data_object();
}

aml_ptr_t parse_def_name()
{
  AML_PRELUDE(NAME_OP)
  printf("DefName ");
  aml_ptr_t name = parse_name_string();
  AML_ERR_CHECK(name);
  print_name_string(name);
  putchar(' ');
  aml_ptr_t object = parse_data_ref_object();
  AML_ERR_CHECK(object);
  printf(" %x ", object.prefix_byte);
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
  AML_PRELUDE(SCOPE_OP)
  uint32_t length      = parse_pkg_length();
  aml_ptr_t scope_name = parse_name_string();
  AML_ERR_CHECK(scope_name);
  printf("Scope %.4s ",
    scope_name.prefix_byte == NULL_NAME ? "Root" : (char*)scope_name.__ptr);
  aml_node_t* scope = calloc(1, sizeof(aml_node_t));
  parse_term_list(length);
  return (aml_ptr_t){SCOPE_OP, scope};
}

aml_ptr_t parse_namespace_modifier_obj()
{
  return one_of(3, parse_def_alias, parse_def_name, parse_def_scope);
}
