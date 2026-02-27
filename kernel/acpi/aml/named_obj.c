#include "aml.h"
#include "parser.h"
#include "stdlib.h"
#include <stdio.h>

aml_ptr_t named_field()
{
  aml_ptr_t name_segment = parse_name_seg();
  if (name_segment.prefix_byte == AML_PREFIX_ERROR) return AML_ERROR;
  parse_pkg_length();
  return (aml_ptr_t){NAME_OP, NULL};
}

aml_ptr_t reserved_field()
{
  AML_PRELUDE(0x00)
  parse_pkg_length();
  return (aml_ptr_t){0x00, NULL};
}

aml_ptr_t access_field()
{
  AML_PRELUDE(0x01)
  next_byte();
  next_byte();
  return (aml_ptr_t){0x01, NULL};
}

aml_ptr_t extended_access_field()
{
  AML_PRELUDE(0x03)
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
  parse_term_arg();
  parse_term_arg();
  parse_name_string();
  return (aml_ptr_t){CREATE_BITFIELD_OP, NULL};
}

aml_ptr_t def_create_bytefield()
{
  AML_PRELUDE(CREATE_BYTEFIELD_OP)
  parse_term_arg();
  parse_term_arg();
  parse_name_string();

  return (aml_ptr_t){CREATE_BYTEFIELD_OP, NULL};
}

aml_ptr_t def_create_dword_field()
{
  AML_PRELUDE(CREATE_DWORDFIELD_OP)
  parse_term_arg();
  parse_term_arg();
  parse_name_string();
  return (aml_ptr_t){CREATE_DWORDFIELD_OP, NULL};
}

aml_ptr_t def_create_field()
{
  AML_EXT_PRELUDE(CREATE_FIELD_OP)
  parse_term_arg();
  parse_term_arg();
  parse_term_arg();
  parse_name_string();
  return (aml_ptr_t){CREATE_FIELD_OP, NULL};
}

aml_ptr_t def_create_qword_field()
{
  AML_PRELUDE(CREATE_QWORDFIELD_OP)
  parse_term_arg();
  parse_term_arg();
  parse_name_string();
  return (aml_ptr_t){CREATE_QWORDFIELD_OP, NULL};
}

aml_ptr_t def_create_word_field()
{
  AML_PRELUDE(CREATE_WORDFIELD_OP)
  parse_term_arg();
  parse_term_arg();
  parse_name_string();
  return (aml_ptr_t){CREATE_WORDFIELD_OP, NULL};
}

aml_ptr_t def_data_region()
{
  AML_EXT_PRELUDE(DATA_REGION_OP)
  parse_name_string();
  parse_term_arg();
  parse_term_arg();
  parse_term_arg();
  return (aml_ptr_t){DATA_REGION_OP, NULL};
}

aml_ptr_t def_external()
{
  AML_PRELUDE(EXTERNAL_OP)
  parse_name_string();
  next_byte();
  next_byte();
  return (aml_ptr_t){EXTERNAL_OP, NULL};
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
  printf("OperationalRegion ");
  aml_ptr_t region_name = parse_name_string();
  uint8_t region_space  = next_byte();
  aml_ptr_t term_arg    = parse_term_arg();
  aml_ptr_t offset      = evaluate_term_arg(term_arg);
  term_arg              = parse_term_arg();
  aml_ptr_t length      = evaluate_term_arg(term_arg);
  print_name_string(region_name);
  putchar(' ');
  print_region_space(region_space);
  printf(" RegionOffset ");
  print_term_arg(offset);
  printf(" RegionLen ");
  print_term_arg(length);
  putchar('\n');
  return (aml_ptr_t){OP_REGION_OP, NULL};
}

aml_ptr_t def_power_res()
{
  AML_EXT_PRELUDE(POWER_RES_OP)
  parse_pkg_length();
  parse_name_string();
  next_byte();
  next_byte();
  next_byte();
  printf("unimplemented");
  abort();
  return (aml_ptr_t){POWER_RES_OP, NULL};
}

aml_ptr_t def_thermal_zone()
{
  AML_EXT_PRELUDE(THERMAL_ZONE_OP)
  parse_pkg_length();
  parse_name_string();
  printf("unimplemented");
  abort();
  return (aml_ptr_t){THERMAL_ZONE_OP, NULL};
}

aml_ptr_t def_field()
{
  AML_EXT_PRELUDE(FIELD_OP)
  uint32_t length       = parse_pkg_length();
  aml_ptr_t region_name = parse_name_string();
  uint8_t field_flags   = next_byte();
  field_list();
  printf("Field %d ", length);
  print_name_string(region_name);
  printf(" %b\n", field_flags);
  return (aml_ptr_t){FIELD_OP, NULL};
}

aml_ptr_t def_method()
{
  AML_PRELUDE(METHOD_OP)
  uint32_t length       = parse_pkg_length();
  aml_ptr_t method_name = parse_name_string();
  uint8_t method_flags  = next_byte();
  printf("Method %d ", length);
  print_name_string(method_name);
  printf(" %b\n", method_flags);
  aml_node_t* method_node = calloc(1, sizeof(aml_node_t));
  parse_term_list(method_node, length);
  return (aml_ptr_t){METHOD_OP, method_node};
}

aml_ptr_t parse_named_obj()
{
  return one_of(14, def_bank_field, def_create_bitfield, def_create_bytefield,
    def_create_dword_field, def_create_field, def_create_qword_field,
    def_create_word_field, def_data_region, def_external, def_op_region,
    def_power_res, def_thermal_zone, def_field, def_method);
}
