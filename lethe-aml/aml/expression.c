#include "aml.h"
#include "parser.h"
#include "stdlib.h"

#include <stdint.h>

static aml_ptr_t def_acquire() { return AML_PREFIX_ERROR; }

static aml_ptr_t def_add() { return AML_PREFIX_ERROR; }

static aml_ptr_t def_and() { return AML_PREFIX_ERROR; }

aml_ptr_t def_buffer()
{
  int       old_pointer = get_pointer();
  uint32_t  length      = parse_pkg_length();
  aml_ptr_t buffer_size = parse_term_arg();
  AML_ERR_CHECK_ABRT(buffer_size);
  int new_pointer = get_pointer();
  length         -= (new_pointer - old_pointer);
  while (length > 0) // TODO use buffer_size instead of pkg_length
    {
      uint8_t byte = next_byte();
      length--;
    }
  return (aml_ptr_t){ BUFFER_OP, NULL };
}

static aml_ptr_t def_concat() { return AML_PREFIX_ERROR; }

static aml_ptr_t def_concat_res() { return AML_PREFIX_ERROR; }

static aml_ptr_t def_cond_ref_of() { return AML_PREFIX_ERROR; }

static aml_ptr_t def_copy_object() { return AML_PREFIX_ERROR; }

static aml_ptr_t def_decrement() { return AML_PREFIX_ERROR; }

static aml_ptr_t def_deref_of() { return AML_PREFIX_ERROR; }

static aml_ptr_t def_divide() { return AML_PREFIX_ERROR; }

static aml_ptr_t def_find_set_left_bit() { return AML_PREFIX_ERROR; }

static aml_ptr_t def_find_set_right_bit() { return AML_PREFIX_ERROR; }

static aml_ptr_t def_from_bcd() { return AML_PREFIX_ERROR; }

static aml_ptr_t def_increment() { return AML_PREFIX_ERROR; }

static aml_ptr_t def_index() { return AML_PREFIX_ERROR; }

static aml_ptr_t def_LAnd() { return AML_PREFIX_ERROR; }

static aml_ptr_t def_LEqual() { return AML_PREFIX_ERROR; }

static aml_ptr_t def_LGreater() { return AML_PREFIX_ERROR; }

static aml_ptr_t def_LGreater_equal() { return AML_PREFIX_ERROR; }

static aml_ptr_t def_LLess() { return AML_PREFIX_ERROR; }

static aml_ptr_t def_LLess_equal() { return AML_PREFIX_ERROR; }

static aml_ptr_t def_mid() { return AML_PREFIX_ERROR; }

static aml_ptr_t def_LNot() { return AML_PREFIX_ERROR; }

static aml_ptr_t def_LNot_equal() { return AML_PREFIX_ERROR; }

static aml_ptr_t def_load_table() { return AML_PREFIX_ERROR; }

static aml_ptr_t def_LOr() { return AML_PREFIX_ERROR; }

static aml_ptr_t def_match() { return AML_PREFIX_ERROR; }

static aml_ptr_t def_mod() { return AML_PREFIX_ERROR; }

static aml_ptr_t def_multiply() { return AML_PREFIX_ERROR; }

static aml_ptr_t def_NAnd() { return AML_PREFIX_ERROR; }

static aml_ptr_t def_NOr() { return AML_PREFIX_ERROR; }

static aml_ptr_t def_not() { return AML_PREFIX_ERROR; }

static aml_ptr_t def_object_type() { return AML_PREFIX_ERROR; }

static aml_ptr_t def_or() { return AML_PREFIX_ERROR; }

// TODO sanity check PkgLength against NumElements
aml_ptr_t def_package()
{
  AML_PRELUDE(PACKAGE_OP);
  int      old_pointer  = get_pointer();
  uint32_t length       = parse_pkg_length();
  uint8_t  num_elements = next_byte();
  int      new_pointer  = get_pointer();
  length               -= (new_pointer - old_pointer);
  aml_ptr_t status;
  for (int i = 0; i < num_elements; i++)
    {
      TRY_PARSE_CONTINUE(parse_data_ref_object);
      TRY_PARSE_CONTINUE(parse_name_string);
    }
  return (aml_ptr_t){ PACKAGE_OP, NULL };
}

aml_ptr_t def_var_package() { return AML_PREFIX_ERROR; }

static aml_ptr_t def_ref_of() { return AML_PREFIX_ERROR; }

static aml_ptr_t def_shift_left() { return AML_PREFIX_ERROR; }

static aml_ptr_t def_shift_right() { return AML_PREFIX_ERROR; }

static aml_ptr_t def_size_of() { return AML_PREFIX_ERROR; }

static aml_ptr_t def_store() { return AML_PREFIX_ERROR; }

static aml_ptr_t def_subtract() { return AML_PREFIX_ERROR; }

static aml_ptr_t def_timer() { return AML_PREFIX_ERROR; }

static aml_ptr_t def_to_bcd() { return AML_PREFIX_ERROR; }

static aml_ptr_t def_to_buffer() { return AML_PREFIX_ERROR; }

static aml_ptr_t def_to_decimal_string() { return AML_PREFIX_ERROR; }

aml_ptr_t reference_type_opcode()
{
  aml_ptr_t status;
  TRY_PARSE(def_ref_of);
  TRY_PARSE(def_deref_of);
  TRY_PARSE(def_index);
  return AML_PREFIX_ERROR;
}

static aml_ptr_t def_to_hex_string() { return AML_PREFIX_ERROR; }

static aml_ptr_t def_to_integer() { return AML_PREFIX_ERROR; }

static aml_ptr_t def_to_string() { return AML_PREFIX_ERROR; }

static aml_ptr_t def_wait() { return AML_PREFIX_ERROR; }

static aml_ptr_t def_xor() { return AML_PREFIX_ERROR; }

static aml_ptr_t method_invocation()
{
  return AML_PREFIX_ERROR;
  aml_ptr_t method_name = parse_name_string();
  if (
    method_name.prefix_byte == ERR_PARSE ||
    method_name.prefix_byte == ERR_PREFIX
  )
    {
      return AML_PREFIX_ERROR;
    }
  aml_ptr_t status = parse_term_arg();
  AML_ERR_CHECK(status);
  while (status.prefix_byte != ERR_PARSE && status.prefix_byte != ERR_PREFIX)
    {
      status = parse_term_arg();
    }
  return (aml_ptr_t){ NULL_NAME, NULL };
}

aml_ptr_t parse_expression_opcode()
{
  uint8_t token = next_byte();
  switch (token)
    {
      case PACKAGE_OP: move_pointer(-1); return def_package();
      case BUFFER_OP: return def_buffer();
      default:
        {
          move_pointer(-1);
          return method_invocation(); // default to method invocation since it
                                      // has no prefix
        }
    }
}
