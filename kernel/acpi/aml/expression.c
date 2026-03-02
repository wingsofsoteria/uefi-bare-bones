#include "aml.h"
#include "parser.h"
#include <iso646.h>
#include <stdint.h>
#include <stdio.h>

aml_ptr_t def_acquire()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_add()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_and()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_buffer()
{
  AML_PRELUDE(BUFFER_OP);
  int old_pointer = get_pointer();
  uint32_t length = parse_pkg_length();
  printf("DefBuffer(%d, ", length);
  aml_ptr_t buffer_size = parse_term_arg();
  AML_ERR_CHECK(buffer_size);
  int new_pointer = get_pointer();
  printf(")");
  length -= (new_pointer - old_pointer);
  while (length > 0) // TODO use buffer_size instead of pkg_length
  {
    uint8_t byte = next_byte();
    length--;
  }
  return (aml_ptr_t){BUFFER_OP, NULL};
}

aml_ptr_t def_concat()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_concat_res()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_cond_ref_of()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_copy_object()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_decrement()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_deref_of()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_divide()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_find_set_left_bit()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_find_set_right_bit()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_from_bcd()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_increment()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_index()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_LAnd()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_LEqual()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_LGreater()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_LGreater_equal()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_LLess()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_LLess_equal()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_mid()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_LNot()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_LNot_equal()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_load_table()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_LOr()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_match()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_mod()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_multiply()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_NAnd()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_NOr()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_not()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_object_type()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_or()
{
  return AML_PREFIX_ERROR;
}

// TODO sanity check PkgLength against NumElements
aml_ptr_t def_package()
{
  AML_PRELUDE(PACKAGE_OP);
  int old_pointer       = get_pointer();
  uint32_t length       = parse_pkg_length();
  uint8_t num_elements  = next_byte();
  int new_pointer       = get_pointer();
  length               -= (new_pointer - old_pointer);
  printf("DefPackage(%d, ", length);
  for (int i = 0; i < num_elements; i++)
  {
    aml_ptr_t status = one_of(2, parse_data_ref_object, parse_name_string);
    AML_ERR_CHECK(status);
    printf(", ");
  }
  return (aml_ptr_t){PACKAGE_OP, NULL};
}

aml_ptr_t def_var_package()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_ref_of()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_shift_left()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_shift_right()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_size_of()
{
  return AML_PREFIX_ERROR;
}
aml_ptr_t def_store()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_subtract()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_timer()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_to_bcd()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_to_buffer()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_to_decimal_string()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t reference_type_opcode()
{
  return one_of(3, def_ref_of, def_deref_of, def_index);
}

aml_ptr_t def_to_hex_string()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_to_integer()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_to_string()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_wait()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_xor()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t method_invocation()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t parse_expression_opcode()
{
  return one_of(53, def_acquire, def_add, def_and, def_buffer, def_concat,
    def_concat_res, def_cond_ref_of, def_copy_object, def_decrement,
    def_deref_of, def_divide, def_find_set_left_bit, def_find_set_right_bit,
    def_from_bcd, def_increment, def_index, def_LAnd, def_LEqual, def_LGreater,
    def_LGreater_equal, def_LLess, def_LLess_equal, def_mid, def_LNot,
    def_LNot_equal, def_load_table, def_LOr, def_match, def_mod, def_multiply,
    def_NAnd, def_NOr, def_not, def_object_type, def_or, def_package,
    def_var_package, def_ref_of, def_shift_left, def_shift_right, def_size_of,
    def_store, def_subtract, def_timer, def_to_bcd, def_to_buffer,
    def_to_decimal_string, def_to_hex_string, def_to_integer, def_to_string,
    def_wait, def_xor, method_invocation);
}
