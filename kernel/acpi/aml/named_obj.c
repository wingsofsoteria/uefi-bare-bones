#include "aml.h"
#include "cpu/isr.h"
#include "parser.h"
#include "stdlib.h"
#include <stdint.h>
#include <stdio.h>

aml_ptr_t named_field()
{
  aml_ptr_t name_seg = parse_name_seg();
  if (name_seg.prefix_byte == ERR_PREFIX || name_seg.prefix_byte == ERR_PARSE)
  {
    return AML_PREFIX_ERROR;
  }
  uint32_t length = parse_pkg_length();
  printf("NamedField(%d, ", length);
  print_name_string(name_seg);
  printf(")");
  return (aml_ptr_t){NAME_OP, NULL};
}
aml_ptr_t reserved_field()
{
  AML_PRELUDE(0x00);
  uint32_t length = parse_pkg_length();
  printf("ReservedField(%d)", length);
  return (aml_ptr_t){0x00, NULL};
}

aml_ptr_t access_field()
{
  AML_PRELUDE(0x01);
  next_byte();
  next_byte();
  printf("AccessField");
  return (aml_ptr_t){0x01, NULL};
}

aml_ptr_t extended_access_field()
{
  AML_PRELUDE(0x03);
  next_byte();
  next_byte();
  next_byte();
  printf("ExtendedAccessField");
  return (aml_ptr_t){0x03, NULL};
}

// BufferData isn't defined in the AML spec so I'm leaving this undefined until
// I find an example of it in my emulator
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
  printf("OperationalRegion(");
  aml_ptr_t region_name = parse_name_string();
  AML_ERR_CHECK(region_name);
  print_name_string(region_name);
  printf(", ");
  uint8_t region_space = next_byte();
  print_region_space(region_space);
  printf(", ");
  aml_ptr_t region_offset = parse_term_arg();
  AML_ERR_CHECK(region_offset);
  printf(", ");
  aml_ptr_t region_len = parse_term_arg();
  AML_ERR_CHECK(region_len);
  printf(");\n");
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
  AML_EXT_PRELUDE(FIELD_OP);
  printf("DefField(");
  int old_pointer       = get_pointer();
  uint32_t length       = parse_pkg_length();
  aml_ptr_t name_string = parse_name_string();
  AML_ERR_CHECK(name_string);
  print_name_string(name_string);
  uint8_t field_flags  = next_byte();
  int new_pointer      = get_pointer();
  length              -= (new_pointer - old_pointer);
  printf(", %d, %d", length, field_flags);
  aml_ptr_t status;
  while (length > 0)
  {
    printf(", ");
    status = one_of(5, named_field, reserved_field, access_field,
      extended_access_field, connect_field);
    AML_ERR_CHECK(status);
    old_pointer = new_pointer;
    new_pointer = get_pointer();
    if ((new_pointer - old_pointer) > length)
    {
      printf("\nOvershot field length\n");
      abort();
    }
    length -= (new_pointer - old_pointer);
  }
  printf(");\n");
  return (aml_ptr_t){FIELD_OP, NULL};
}

aml_ptr_t def_device()
{
  AML_EXT_PRELUDE(DEVICE_OP);
  printf("DefDevice(");
  int current_pointer   = get_pointer();
  uint32_t length       = parse_pkg_length();
  aml_ptr_t name_string = parse_name_string();
  AML_ERR_CHECK(name_string);
  print_name_string(name_string);
  printf(", ");
  int new_pointer      = get_pointer();
  length              -= (new_pointer - current_pointer);
  aml_ptr_t term_list  = parse_term_list(length);
  AML_ERR_CHECK(term_list);
  printf(");\n");
  return (aml_ptr_t){DEVICE_OP, NULL};
}
// TODO replace move_pointer with actual parser code
aml_ptr_t def_method()
{
  AML_PRELUDE(METHOD_OP);
  printf("DefMethod(");
  int current_pointer   = get_pointer();
  uint32_t length       = parse_pkg_length();
  aml_ptr_t name_string = parse_name_string();
  AML_ERR_CHECK(name_string);
  print_name_string(name_string);
  printf(", %d", length);
  uint8_t method_flags  = next_byte();
  int new_pointer       = get_pointer();
  length               -= (new_pointer - current_pointer);
  move_pointer(length);
  printf(");\n");
  return (aml_ptr_t){METHOD_OP, NULL};
}

aml_ptr_t def_mutex()
{
  AML_EXT_PRELUDE(MUTEX_OP);
  aml_ptr_t name_string = parse_name_string();
  AML_ERR_CHECK(name_string);
  uint8_t sync_flags = next_byte();
  printf("DefMutex(");
  print_name_string(name_string);
  printf(", %d);\n", sync_flags);
  return (aml_ptr_t){MUTEX_OP, NULL};
}

// Compatibility for outdated ACPI versions
aml_ptr_t def_processor()
{
  AML_EXT_PRELUDE(0x83);
  printf("DEPRECATED\n");
  int old_pointer = get_pointer();
  uint32_t length = parse_pkg_length();
  parse_name_string();
  next_byte();

  next_byte();
  next_byte();
  next_byte();
  next_byte();

  next_byte();
  int new_pointer = get_pointer();
  move_pointer(length - (new_pointer - old_pointer));
  return (aml_ptr_t){0x83, NULL};
}

aml_ptr_t parse_named_obj()
{
  return one_of(17, def_processor, def_mutex, def_device, def_bank_field,
    def_create_bitfield, def_create_bytefield, def_create_dword_field,
    def_create_field, def_create_qword_field, def_create_word_field,
    def_data_region, def_external, def_op_region, def_power_res,
    def_thermal_zone, def_field, def_method);
}
