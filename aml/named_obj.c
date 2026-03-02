#include "aml.h"
#include "host.h"
#include "parser.h"

aml_ptr_t named_field()
{
  aml_ptr_t name_seg = parse_name_seg();
  if (name_seg.prefix_byte == ERR_PREFIX || name_seg.prefix_byte == ERR_PARSE)
  {
    return AML_PREFIX_ERROR;
  }
  uint32_t length = parse_pkg_length();
  return (aml_ptr_t){NAME_OP, NULL};
}
aml_ptr_t reserved_field()
{
  AML_PRELUDE(0x00);
  uint32_t length = parse_pkg_length();
  return (aml_ptr_t){0x00, NULL};
}

aml_ptr_t access_field()
{
  AML_PRELUDE(0x01);
  next_byte();
  next_byte();
  return (aml_ptr_t){0x01, NULL};
}

aml_ptr_t extended_access_field()
{
  AML_PRELUDE(0x03);
  next_byte();
  next_byte();
  next_byte();
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
        AML_LOG("SystemMemory");
        break;
      }
    case 0x01:
      {
        AML_LOG("SystemIO");
        break;
      }
    case 0x02:
      {
        AML_LOG("PCI_Config");
        break;
      }
    case 0x03:
      {
        AML_LOG("EmbeddedControl");
        break;
      }
    case 0x04:
      {
        AML_LOG("SMBus");
        break;
      }
    case 0x05:
      {
        AML_LOG("System CMOS");
        break;
      }
    case 0x06:
      {
        AML_LOG("PciBarTarget");
        break;
      }
    case 0x07:
      {
        AML_LOG("IPMI");
        break;
      }
    case 0x08:
      {
        AML_LOG("GeneralPurposeIO");
        break;
      }
    case 0x09:
      {
        AML_LOG("GenericSerialBus");
        break;
      }
    case 0x0A:
      {
        AML_LOG("PCC");
        break;
      }
    default:
      {
        AML_LOG("OEM Defined");
        break;
      }
  }
}

aml_ptr_t def_op_region()
{
  AML_EXT_PRELUDE(OP_REGION_OP);
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
  AML_EXT_PRELUDE(FIELD_OP);
  int old_pointer       = get_pointer();
  uint32_t length       = parse_pkg_length();
  aml_ptr_t name_string = parse_name_string();
  AML_ERR_CHECK(name_string);
  uint8_t field_flags  = next_byte();
  int new_pointer      = get_pointer();
  length              -= (new_pointer - old_pointer);
  aml_ptr_t status;
  while (length > 0)
  {
    status = one_of(5, named_field, reserved_field, access_field,
      extended_access_field, connect_field);
    AML_ERR_CHECK(status);
    old_pointer = new_pointer;
    new_pointer = get_pointer();
    if ((new_pointer - old_pointer) > length)
    {
      AML_EXIT();
    }
    length -= (new_pointer - old_pointer);
  }
  return (aml_ptr_t){FIELD_OP, NULL};
}

aml_ptr_t def_device()
{
  AML_EXT_PRELUDE(DEVICE_OP);
  int current_pointer   = get_pointer();
  uint32_t length       = parse_pkg_length();
  aml_ptr_t name_string = parse_name_string();
  AML_ERR_CHECK(name_string);
  int new_pointer      = get_pointer();
  length              -= (new_pointer - current_pointer);
  aml_ptr_t term_list  = parse_term_list(length);
  AML_ERR_CHECK(term_list);
  return (aml_ptr_t){DEVICE_OP, NULL};
}
// TODO replace move_pointer with actual parser code
aml_ptr_t def_method()
{
  AML_PRELUDE(METHOD_OP);
  int current_pointer   = get_pointer();
  uint32_t length       = parse_pkg_length();
  aml_ptr_t name_string = parse_name_string();
  AML_ERR_CHECK(name_string);
  uint8_t method_flags  = next_byte();
  int new_pointer       = get_pointer();
  length               -= (new_pointer - current_pointer);
  move_pointer(length);
  return (aml_ptr_t){METHOD_OP, NULL};
}

aml_ptr_t def_mutex()
{
  AML_EXT_PRELUDE(MUTEX_OP);
  aml_ptr_t name_string = parse_name_string();
  AML_ERR_CHECK(name_string);
  uint8_t sync_flags = next_byte();
  return (aml_ptr_t){MUTEX_OP, NULL};
}

// Compatibility for outdated ACPI versions
aml_ptr_t def_processor()
{
  AML_EXT_PRELUDE(0x83);
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
