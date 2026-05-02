#include "aml.h"
#include "parser.h"
#include "stdlib.h"

static aml_ptr_t def_break() { return AML_PREFIX_ERROR; }

static aml_ptr_t def_breakpoint() { return AML_PREFIX_ERROR; }

static aml_ptr_t def_continue() { return AML_PREFIX_ERROR; }

static aml_ptr_t def_fatal() { return AML_PREFIX_ERROR; }

static aml_ptr_t def_else() { return AML_PREFIX_ERROR; }

static aml_ptr_t def_if_else()
{
  int       old_pointer = get_pointer();
  uint32_t  length      = parse_pkg_length();
  aml_ptr_t term_arg    = parse_term_arg();
  AML_ERR_CHECK_ABRT(term_arg);
  int new_pointer = get_pointer();
  move_pointer(length - (new_pointer - old_pointer));
  aml_ptr_t status;
  TRY_PARSE(def_else);
  // aml_ptr_t _else = one_of(NULL, 1, def_else);

  return (aml_ptr_t){ IF_OP, NULL };
}

static aml_ptr_t def_noop() { return AML_PREFIX_ERROR; }

static aml_ptr_t def_notify() { return AML_PREFIX_ERROR; }

static aml_ptr_t def_release() { return AML_PREFIX_ERROR; }

static aml_ptr_t def_reset() { return AML_PREFIX_ERROR; }

static aml_ptr_t def_return() { return AML_PREFIX_ERROR; }

static aml_ptr_t def_signal() { return AML_PREFIX_ERROR; }

static aml_ptr_t def_sleep() { return AML_PREFIX_ERROR; }

static aml_ptr_t def_stall() { return AML_PREFIX_ERROR; }

static aml_ptr_t def_while() { return AML_PREFIX_ERROR; }

aml_ptr_t parse_statement_opcode()
{
  uint8_t token = next_byte();
  switch (token)
    {
      case IF_OP: return def_if_else();
      default: return AML_PREFIX_ERROR;
    }
}
