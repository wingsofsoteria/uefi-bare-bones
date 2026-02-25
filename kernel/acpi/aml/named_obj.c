#include "aml.h"
#include "parser.h"
#include "stdlib.h"
#include <stdio.h>

aml_ptr_t def_bank_field()
{
  return AML_ERROR;
}

aml_ptr_t def_create_bitfield()
{
  return AML_ERROR;
}

aml_ptr_t def_create_bytefield()
{
  return AML_ERROR;
}

aml_ptr_t def_create_dword_field()
{
  return AML_ERROR;
}

aml_ptr_t def_create_field()
{
  return AML_ERROR;
}

aml_ptr_t def_create_qword_field()
{
  return AML_ERROR;
}

aml_ptr_t def_create_word_field()
{
  return AML_ERROR;
}

aml_ptr_t def_data_region()
{
  return AML_ERROR;
}

aml_ptr_t def_external()
{
  return AML_ERROR;
}

aml_ptr_t def_op_region()
{
  uint8_t token = next_byte();
  if (token != EXT_OP_PREFIX) return AML_ERROR;
  token = next_byte();
  if (token != OP_REGION_OP) return AML_ERROR;
  printf("DefOpRegion\n");
  parse_name_string();
  next_byte();      // region space
  parse_term_arg(); // region offset
  parse_term_arg(); // region len
  return (aml_ptr_t){OP_REGION_OP, NULL};
}

aml_ptr_t def_power_res()
{
  return AML_ERROR;
}

aml_ptr_t def_thermal_zone()
{
  return AML_ERROR;
}

aml_ptr_t parse_named_obj()
{
  printf("NamedObj\n");
  return one_of(12, def_bank_field, def_create_bitfield, def_create_bytefield,
    def_create_dword_field, def_create_field, def_create_qword_field,
    def_create_word_field, def_data_region, def_external, def_op_region,
    def_power_res, def_thermal_zone);
}
