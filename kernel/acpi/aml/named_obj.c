#include "aml.h"
#include "parser.h"
#include "stdlib.h"
#include <stdio.h>

aml_ptr_t named_field()
{
  return AML_PREFIX_ERROR;
}
aml_ptr_t reserved_field()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t access_field()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t extended_access_field()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t connect_field()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t field_list()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_bank_field()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_create_bitfield()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_create_bytefield()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_create_dword_field()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_create_field()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_create_qword_field()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_create_word_field()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_data_region()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_external()
{
  return AML_PREFIX_ERROR;
}

void print_region_space(uint8_t region_space)
{
  switch (region_space)
  {
    case 0x00:
      {
        printf("SystemMemory");
        break;
      }
    case 0x01:
      {
        printf("SystemIO");
        break;
      }
    case 0x02:
      {
        printf("PCI_Config");
        break;
      }
    case 0x03:
      {
        printf("EmbeddedControl");
        break;
      }
    case 0x04:
      {
        printf("SMBus");
        break;
      }
    case 0x05:
      {
        printf("System CMOS");
        break;
      }
    case 0x06:
      {
        printf("PciBarTarget");
        break;
      }
    case 0x07:
      {
        printf("IPMI");
        break;
      }
    case 0x08:
      {
        printf("GeneralPurposeIO");
        break;
      }
    case 0x09:
      {
        printf("GenericSerialBus");
        break;
      }
    case 0x0A:
      {
        printf("PCC");
        break;
      }
    default:
      {
        printf("OEM Defined");
        break;
      }
  }
}

aml_ptr_t def_op_region()
{
  AML_EXT_PRELUDE(OP_REGION_OP);
  printf("DefOpRegion ");
  aml_ptr_t region_name = parse_name_string();
  AML_ERR_CHECK(region_name);
  uint8_t region_space    = next_byte();
  aml_ptr_t region_offset = parse_term_arg();
  AML_ERR_CHECK(region_offset);
  aml_ptr_t region_len = parse_term_arg();
  AML_ERR_CHECK(region_len);

  return (aml_ptr_t){OP_REGION_OP, NULL};
}

aml_ptr_t def_power_res()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_thermal_zone()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_field()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_device()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_method()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_mutex()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t parse_named_obj()
{
  return one_of(16, def_mutex, def_device, def_bank_field, def_create_bitfield,
    def_create_bytefield, def_create_dword_field, def_create_field,
    def_create_qword_field, def_create_word_field, def_data_region,
    def_external, def_op_region, def_power_res, def_thermal_zone, def_field,
    def_method);
}
