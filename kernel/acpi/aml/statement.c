#include "aml.h"
#include "parser.h"
#include <stdint.h>

aml_ptr_t def_break()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_breakpoint()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_continue()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_fatal()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_else()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_if_else()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_noop()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_notify()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_release()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_reset()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_return()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_signal()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_sleep()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_stall()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t def_while()
{
  return AML_PREFIX_ERROR;
}

aml_ptr_t parse_statement_opcode()
{
  return one_of(14, def_break, def_breakpoint, def_continue, def_fatal,
    def_if_else, def_noop, def_notify, def_release, def_reset, def_return,
    def_signal, def_sleep, def_stall, def_while);
}
