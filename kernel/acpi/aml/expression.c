#include "aml.h"
#include "parser.h"
#include "stdlib.h"
#include <stdio.h>

aml_ptr_t def_acquire()
{
  return AML_ERROR;
}

aml_ptr_t def_add()
{
  return AML_ERROR;
}

aml_ptr_t def_and()
{
  return AML_ERROR;
}

aml_ptr_t def_buffer()
{
  AML_PRELUDE(BUFFER_OP)
  printf("DefBuffer(%d) ", get_pointer());
  parse_pkg_length();
  uint32_t length    = 0;
  aml_ptr_t term_arg = parse_term_arg();
  switch (term_arg.prefix_byte)
  {
    case ONE_OP:
      {
        length = 1;
        break;
      }
    case BYTE_PREFIX:
      {
        uint8_t value = *(uint8_t*)term_arg.__ptr;
        length        = value;
        break;
      }
    case WORD_PREFIX:
      {
        uint16_t value = *(uint16_t*)term_arg.__ptr;
        length         = value;
        break;
      }
    default:
      {
        abort();
      }
  }
  while (length > 0)
  {
    next_byte();
    length--;
  }
  return (aml_ptr_t){BUFFER_OP, NULL};
}

aml_ptr_t def_concat()
{
  return AML_ERROR;
}

aml_ptr_t def_concat_res()
{
  return AML_ERROR;
}

aml_ptr_t def_cond_ref_of()
{
  return AML_ERROR;
}

aml_ptr_t def_copy_object()
{
  return AML_ERROR;
}

aml_ptr_t def_decrement()
{
  return AML_ERROR;
}

aml_ptr_t def_deref_of()
{
  AML_PRELUDE(DEREF_OF_OP)
  printf("DefDerefOfOp(%d) ", get_pointer());
  parse_term_arg();
  parse_term_arg();
  return (aml_ptr_t){DEREF_OF_OP, NULL};
}

aml_ptr_t def_divide()
{
  return AML_ERROR;
}

aml_ptr_t def_find_set_left_bit()
{
  return AML_ERROR;
}

aml_ptr_t def_find_set_right_bit()
{
  return AML_ERROR;
}

aml_ptr_t def_from_bcd()
{
  return AML_ERROR;
}

aml_ptr_t def_increment()
{
  AML_PRELUDE(INCREMENT_OP)
  printf("DefIncrement(%d) ", get_pointer());
  parse_super_name();
  return (aml_ptr_t){INCREMENT_OP, NULL};
}

aml_ptr_t def_index()
{
  AML_PRELUDE(INDEX_OP)
  printf("DefIndex(%d) ", get_pointer());
  parse_term_arg();
  parse_term_arg();
  parse_target();
  return (aml_ptr_t){INDEX_OP, NULL};
}

aml_ptr_t def_LAnd()
{
  return AML_ERROR;
}

aml_ptr_t def_LEqual()
{
  AML_PRELUDE(LEQUAL_OP)
  printf("DefLEqual(%d) ", get_pointer());
  parse_term_arg();
  parse_term_arg();
  return (aml_ptr_t){LEQUAL_OP, NULL};
}

aml_ptr_t def_LGreater()
{
  return AML_ERROR;
}

aml_ptr_t def_LGreater_equal()
{
  return AML_ERROR;
}

aml_ptr_t def_LLess()
{
  AML_PRELUDE(LLESS_OP)
  printf("DefLLess(%d) ", get_pointer());
  parse_term_arg();
  parse_term_arg();
  return (aml_ptr_t){LLESS_OP, NULL};
}

aml_ptr_t def_LLess_equal()
{
  return AML_ERROR;
}

aml_ptr_t def_mid()
{
  return AML_ERROR;
}

aml_ptr_t def_LNot()
{
  return AML_ERROR;
}

aml_ptr_t def_LNot_equal()
{
  return AML_ERROR;
}

aml_ptr_t def_load_table()
{
  return AML_ERROR;
}

aml_ptr_t def_LOr()
{
  return AML_ERROR;
}

aml_ptr_t def_match()
{
  return AML_ERROR;
}

aml_ptr_t def_mod()
{
  return AML_ERROR;
}

aml_ptr_t def_multiply()
{
  return AML_ERROR;
}

aml_ptr_t def_NAnd()
{
  return AML_ERROR;
}

aml_ptr_t def_NOr()
{
  return AML_ERROR;
}

aml_ptr_t def_not()
{
  return AML_ERROR;
}

aml_ptr_t def_object_type()
{
  return AML_ERROR;
}

aml_ptr_t def_or()
{
  return AML_ERROR;
}

aml_ptr_t def_package()
{
  return AML_ERROR;
}

aml_ptr_t def_var_package()
{
  return AML_ERROR;
}

aml_ptr_t def_ref_of()
{
  return AML_ERROR;
}

aml_ptr_t def_shift_left()
{
  return AML_ERROR;
}

aml_ptr_t def_shift_right()
{
  return AML_ERROR;
}

aml_ptr_t def_size_of()
{
  AML_PRELUDE(SIZEOF_OP)
  printf("DefSizeOf(%d) ", get_pointer());
  aml_ptr_t operand = parse_super_name();
  return (aml_ptr_t){SIZEOF_OP, NULL};
}

aml_ptr_t def_store()
{
  AML_PRELUDE(STORE_OP)
  printf("DefStore(%d) ", get_pointer());
  parse_term_arg();
  aml_ptr_t name = parse_super_name();
  return (aml_ptr_t){STORE_OP, NULL};
}

aml_ptr_t def_subtract()
{
  AML_PRELUDE(SUBTRACT_OP)
  printf("DefSubtract(%d) ", get_pointer());
  aml_ptr_t term_arg = parse_term_arg();
  term_arg           = parse_term_arg();
  aml_ptr_t result   = parse_target();
  return (aml_ptr_t){SUBTRACT_OP, NULL};
}

aml_ptr_t def_timer()
{
  return AML_ERROR;
}

aml_ptr_t def_to_bcd()
{
  return AML_ERROR;
}

aml_ptr_t def_to_buffer()
{
  AML_PRELUDE(TO_BUFFER_OP)
  printf("DefToBuffer(%d) ", get_pointer());
  aml_ptr_t term_arg = parse_term_arg();
  aml_ptr_t operand  = evaluate_term_arg(term_arg);
  aml_ptr_t target   = parse_target();
  // we're supposed to do some manipulation of data here but I'm leaving it
  // unimplemented until it becomes an issue
  return (aml_ptr_t){TO_BUFFER_OP, NULL};
}

aml_ptr_t def_to_decimal_string()
{
  return AML_ERROR;
}

aml_ptr_t reference_type_opcode()
{
  return one_of(3, def_ref_of, def_deref_of, def_index);
}

aml_ptr_t def_to_hex_string()
{
  AML_PRELUDE(TO_HEX_STRING_OP)
  printf("DefToHexString(%d) ", get_pointer());
  aml_ptr_t term_arg = parse_term_arg();
  aml_ptr_t target   = parse_target();
  return (aml_ptr_t){TO_HEX_STRING_OP, NULL};
}

aml_ptr_t def_to_integer()
{
  return AML_ERROR;
}

aml_ptr_t def_to_string()
{
  return AML_ERROR;
}

aml_ptr_t def_wait()
{
  return AML_ERROR;
}

aml_ptr_t def_xor()
{
  return AML_ERROR;
}

aml_ptr_t method_invocation()
{
  return AML_ERROR;
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
