#include "aml.h"
#include "host.h"
#include "parser.h"
#include "stdlib.h"

static aml_ptr_t named_field()
{
  move_pointer(4);
  uint32_t length = parse_pkg_length();
  return (aml_ptr_t){NAME_OP, NULL};
}
static aml_ptr_t reserved_field()
{
  uint32_t length = parse_pkg_length();
  return (aml_ptr_t){0x00, NULL};
}

static aml_ptr_t access_field()
{
  next_byte();
  next_byte();
  return (aml_ptr_t){0x01, NULL};
}

static aml_ptr_t extended_access_field()
{
  next_byte();
  next_byte();
  next_byte();
  return (aml_ptr_t){0x03, NULL};
}

// BufferData isn't defined in the AML spec so I'm leaving this undefined until
// I find an example of it in my emulator
static aml_ptr_t connect_field()
{
  return AML_PREFIX_ERROR;
}

static aml_ptr_t field_list()
{
  return AML_PREFIX_ERROR;
}

static aml_ptr_t def_bank_field()
{
  return AML_PREFIX_ERROR;
}

static aml_ptr_t def_create_bitfield()
{
  return AML_PREFIX_ERROR;
}

static aml_ptr_t def_create_bytefield()
{
  return AML_PREFIX_ERROR;
}

static aml_ptr_t def_create_dword_field()
{
  return AML_PREFIX_ERROR;
}

static aml_ptr_t def_create_field()
{
  return AML_PREFIX_ERROR;
}

static aml_ptr_t def_create_qword_field()
{
  return AML_PREFIX_ERROR;
}

static aml_ptr_t def_create_word_field()
{
  return AML_PREFIX_ERROR;
}

static aml_ptr_t def_data_region()
{
  return AML_PREFIX_ERROR;
}

static aml_ptr_t def_external()
{
  return AML_PREFIX_ERROR;
}

static void print_region_space(uint8_t region_space)
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

static aml_ptr_t def_op_region()
{
  aml_ptr_t region_name = parse_name_string();
  AML_ERR_CHECK_ABRT(region_name);

  uint8_t region_space    = next_byte();
  aml_ptr_t region_offset = parse_term_arg();
  AML_SET_ANCHOR;
  AML_ERR_CHECK_ABRT(region_offset);
  aml_ptr_t region_len = parse_term_arg();
  AML_ERR_CHECK_ABRT(region_len);
  return (aml_ptr_t){EXT_OP_REGION_OP, NULL};
}

static aml_ptr_t def_power_res()
{
  return AML_PREFIX_ERROR;
}

static aml_ptr_t def_thermal_zone()
{
  return AML_PREFIX_ERROR;
}

static aml_ptr_t def_field()
{
  int old_pointer       = get_pointer();
  uint32_t length       = parse_pkg_length();
  aml_ptr_t name_string = parse_name_string();
  AML_ERR_CHECK_ABRT(name_string);
  uint8_t field_flags  = next_byte();
  int new_pointer      = get_pointer();
  length              -= (new_pointer - old_pointer);
  aml_ptr_t status;
  while ((get_pointer() - new_pointer) < length)
  {
    TRY_PARSE_CONTINUE(named_field);
    TRY_PARSE_CONTINUE(reserved_field);
    TRY_PARSE_CONTINUE(access_field);
    TRY_PARSE_CONTINUE(extended_access_field);
    TRY_PARSE_CONTINUE(connect_field);
    AML_ERR_CHECK_ABRT(status);
  }
  return (aml_ptr_t){EXT_FIELD_OP, NULL};
}

static aml_ptr_t def_device(void* map_ptr)
{
  int current_pointer   = get_pointer();
  uint32_t length       = parse_pkg_length();
  aml_ptr_t name_string = parse_name_string();
  AML_ERR_CHECK_ABRT(name_string);
  int new_pointer      = get_pointer();
  length              -= (new_pointer - current_pointer);
  aml_ptr_t term_list  = parse_term_list(map_ptr, length);
  AML_ERR_CHECK_ABRT(term_list);
  return (aml_ptr_t){EXT_DEVICE_OP, NULL};
}
// TODO replace move_pointer with actual parser code
static aml_ptr_t def_method()
{
  int current_pointer   = get_pointer();
  uint32_t length       = parse_pkg_length();
  aml_ptr_t name_string = parse_name_string();
  AML_ERR_CHECK_ABRT(name_string);
  uint8_t method_flags  = next_byte();
  int new_pointer       = get_pointer();
  length               -= (new_pointer - current_pointer);
  move_pointer(length);
  return (aml_ptr_t){METHOD_OP, NULL};
}

static aml_ptr_t def_mutex()
{
  aml_ptr_t name_string = parse_name_string();
  AML_ERR_CHECK_ABRT(name_string);
  uint8_t sync_flags = next_byte();
  return (aml_ptr_t){EXT_MUTEX_OP, NULL};
}

// Compatibility for outdated ACPI versions
static aml_ptr_t def_processor()
{
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

static aml_ptr_t def_index_field()
{
  int old_pointer      = get_pointer();
  uint32_t length      = parse_pkg_length();
  aml_ptr_t index_name = parse_name_string();
  AML_ERR_CHECK_ABRT(index_name);
  aml_ptr_t data_name = parse_name_string();
  AML_ERR_CHECK_ABRT(data_name);
  uint8_t field_flags  = next_byte();
  int new_pointer      = get_pointer();
  length              -= (new_pointer - old_pointer);
  aml_ptr_t status;
  while ((get_pointer() - new_pointer) < length)
  {
    TRY_PARSE_CONTINUE(named_field);
    TRY_PARSE_CONTINUE(reserved_field);
    TRY_PARSE_CONTINUE(access_field);
    TRY_PARSE_CONTINUE(extended_access_field);
    TRY_PARSE_CONTINUE(connect_field);
    AML_ERR_CHECK_ABRT(status);
  }
  return (aml_ptr_t){EXT_INDEX_FIELD_OP, NULL};
}

static aml_ptr_t def_event()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t parse_named_obj(void* map_ptr)
{
  aml_ptr_t status;
  uint8_t token = next_byte();
  switch (token)
  {
    case EXT_OP_PREFIX:
      {
        token = next_byte();
        switch (token)
        {
          case EXT_BANK_FIELD_OP:
            return def_bank_field();
          case EXT_CREATE_FIELD_OP:
            return def_create_field();
          case EXT_DATA_REGION_OP:
            return def_data_region();
          case EXT_DEVICE_OP:
            return def_device(map_ptr);
          case EXT_EVENT_OP:
            return def_event();
          case EXT_FIELD_OP:
            return def_field();
          case EXT_INDEX_FIELD_OP:
            return def_index_field();
          case EXT_MUTEX_OP:
            return def_mutex();
          case EXT_OP_REGION_OP:
            return def_op_region();
          case EXT_POWER_RES_OP:
            return def_power_res();
          case EXT_THERMAL_ZONE_OP:
            return def_thermal_zone();
          case 0x83:
            return def_processor();
          default:
            {
              return AML_PREFIX_ERROR;
            }
        }
      }
    case METHOD_OP:
      return def_method();
    case EXTERNAL_OP:
      return def_external();
    case CREATE_BITFIELD_OP:
      return def_create_bitfield();
    case CREATE_BYTEFIELD_OP:
      return def_create_bytefield();
    case CREATE_DWORDFIELD_OP:
      return def_create_dword_field();
    case CREATE_QWORDFIELD_OP:
      return def_create_qword_field();
    case CREATE_WORDFIELD_OP:
      return def_create_word_field();
    default:
      {
        return AML_PREFIX_ERROR;
      }
  }
}
