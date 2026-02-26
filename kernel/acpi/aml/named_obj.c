#include "aml.h"
#include "parser.h"
#include "stdlib.h"
#include <stdio.h>

aml_ptr_t named_field()
{
  aml_ptr_t name_segment = parse_name_seg();
  if (name_segment.prefix_byte == AML_PREFIX_ERROR) return AML_ERROR;
  printf("NamedField %.4s ",
    (char*)name_segment.__ptr); // TODO this might cause weird errors
  parse_pkg_length();
  return (aml_ptr_t){NAME_OP, NULL};
}

aml_ptr_t reserved_field()
{
  AML_PRELUDE(0x00)
  printf("ReservedField ");
  parse_pkg_length();
  return (aml_ptr_t){0x00, NULL};
}

aml_ptr_t access_field()
{
  AML_PRELUDE(0x01)
  printf("AccessField ");
  next_byte();
  next_byte();
  return (aml_ptr_t){0x01, NULL};
}

aml_ptr_t extended_access_field()
{
  AML_PRELUDE(0x03)
  printf("ExtendedAccessField ");
  next_byte();
  next_byte();
  next_byte(); // TODO AccessLength is not fully defined in the AML Spec but it
               // is labeled as ByteConst in the ASL spec
  return (aml_ptr_t){0x03, NULL};
}

aml_ptr_t connect_field()
{
  return AML_ERROR; // TODO parse connect field properly
}

aml_ptr_t field_list()
{
  aml_ptr_t status = {FIELD_OP, NULL};
  while (status.prefix_byte != AML_PREFIX_ERROR)
  {
    status = one_of(5, reserved_field, access_field, extended_access_field,
      connect_field, named_field);
  }
  return (aml_ptr_t){FIELD_OP, NULL};
}

aml_ptr_t def_bank_field()
{
  AML_EXT_PRELUDE(BANK_FIELD_OP)
  printf("BankField ");
  parse_pkg_length();
  aml_ptr_t region_name = parse_name_string();
  aml_ptr_t bank_name   = parse_name_string();
  parse_term_arg();
  next_byte();

  return field_list();
}

aml_ptr_t def_create_bitfield()
{
  AML_PRELUDE(CREATE_BITFIELD_OP)
  printf("CreateBitfield ");
  parse_term_arg();
  parse_term_arg();
  parse_name_string();
  return (aml_ptr_t){CREATE_BITFIELD_OP, NULL};
}

aml_ptr_t def_create_bytefield()
{
  AML_PRELUDE(CREATE_BYTEFIELD_OP)
  printf("CreateByteField ");
  parse_term_arg();
  parse_term_arg();
  parse_name_string();

  return (aml_ptr_t){CREATE_BYTEFIELD_OP, NULL};
}

aml_ptr_t def_create_dword_field()
{
  AML_PRELUDE(CREATE_DWORDFIELD_OP)
  printf("CreateDWORDField ");
  parse_term_arg();
  parse_term_arg();
  parse_name_string();
  return (aml_ptr_t){CREATE_DWORDFIELD_OP, NULL};
}

aml_ptr_t def_create_field()
{
  AML_EXT_PRELUDE(CREATE_FIELD_OP)
  printf("CreateField ");
  parse_term_arg();
  parse_term_arg();
  parse_term_arg();
  parse_name_string();
  return (aml_ptr_t){CREATE_FIELD_OP, NULL};
}

aml_ptr_t def_create_qword_field()
{
  AML_PRELUDE(CREATE_QWORDFIELD_OP)
  printf("CreateQWORDField ");
  parse_term_arg();
  parse_term_arg();
  parse_name_string();
  return (aml_ptr_t){CREATE_QWORDFIELD_OP, NULL};
}

aml_ptr_t def_create_word_field()
{
  AML_PRELUDE(CREATE_WORDFIELD_OP)
  printf("CreateWORDField ");
  parse_term_arg();
  parse_term_arg();
  parse_name_string();
  return (aml_ptr_t){CREATE_WORDFIELD_OP, NULL};
}

aml_ptr_t def_data_region()
{
  AML_EXT_PRELUDE(DATA_REGION_OP)
  printf("DefDataRegion ");
  parse_name_string();
  parse_term_arg();
  parse_term_arg();
  parse_term_arg();
  return (aml_ptr_t){DATA_REGION_OP, NULL};
}

aml_ptr_t def_external()
{
  AML_PRELUDE(EXTERNAL_OP)
  printf("DefExternal ");
  parse_name_string();
  next_byte();
  next_byte();
  return (aml_ptr_t){EXTERNAL_OP, NULL};
}

aml_ptr_t def_op_region()
{
  AML_EXT_PRELUDE(OP_REGION_OP);
  printf("DefOpRegion ");
  parse_name_string();
  next_byte();      // region space
  parse_term_arg(); // region offset
  parse_term_arg(); // region len
  return (aml_ptr_t){OP_REGION_OP, NULL};
}

aml_ptr_t def_power_res()
{
  AML_EXT_PRELUDE(POWER_RES_OP)
  printf("DefPowerRes ");
  parse_pkg_length();
  parse_name_string();
  next_byte();
  next_byte();
  next_byte();
  parse_term_list();
  return (aml_ptr_t){POWER_RES_OP, NULL};
}

aml_ptr_t def_thermal_zone()
{
  AML_EXT_PRELUDE(THERMAL_ZONE_OP)
  printf("DefThermalZone ");
  parse_pkg_length();
  parse_name_string();
  parse_term_list();
  return (aml_ptr_t){THERMAL_ZONE_OP, NULL};
}

aml_ptr_t def_field()
{
  AML_EXT_PRELUDE(FIELD_OP)
  printf("DefField ");
  parse_pkg_length();
  parse_name_string();
  next_byte();
  field_list();
  return (aml_ptr_t){FIELD_OP, NULL};
}

aml_ptr_t def_method()
{
  AML_PRELUDE(METHOD_OP)
  printf("DefMethod ");
  parse_pkg_length();
  parse_name_string();
  next_byte();
  parse_term_list();
  return (aml_ptr_t){METHOD_OP, NULL};
}

aml_ptr_t parse_named_obj()
{
  return one_of(14, def_bank_field, def_create_bitfield, def_create_bytefield,
    def_create_dword_field, def_create_field, def_create_qword_field,
    def_create_word_field, def_data_region, def_external, def_op_region,
    def_power_res, def_thermal_zone, def_field, def_method);
}
