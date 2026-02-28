#include "aml.h"
#include "parser.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

aml_ptr_t def_break()
{
  return AML_ERROR;
}

aml_ptr_t def_breakpoint()
{
  return AML_ERROR;
}

aml_ptr_t def_continue()
{
  return AML_ERROR;
}

aml_ptr_t def_fatal()
{
  return AML_ERROR;
}

aml_ptr_t def_else()
{
  AML_PRELUDE(ELSE_OP)
  printf("ElseStatement(%d) ", get_pointer());
  uint32_t pkg_length   = parse_pkg_length();
  aml_node_t* else_node = calloc(1, sizeof(aml_node_t));
  parse_term_list(else_node, pkg_length);
  return (aml_ptr_t){ELSE_OP, else_node};
}

aml_ptr_t def_if_else()
{
  AML_PRELUDE(IF_OP)
  printf("IfStatement(%d) ", get_pointer());
  uint32_t pkg_length = parse_pkg_length();
  parse_term_arg();
  aml_node_t* if_node = calloc(1, sizeof(aml_node_t));
  parse_term_list(if_node, pkg_length);
  aml_ptr_t else_op = def_else();
  if (else_op.prefix_byte != AML_PREFIX_ERROR)
  {
    ((aml_node_t*)else_op.__ptr)->parent = if_node;
    return (aml_ptr_t){IF_OP, else_op.__ptr};
  }
  return (aml_ptr_t){IF_OP, if_node};
}

aml_ptr_t def_noop()
{
  return AML_ERROR;
}

aml_ptr_t def_notify()
{
  return AML_ERROR;
}

aml_ptr_t def_release()
{
  return AML_ERROR;
}

aml_ptr_t def_reset()
{
  return AML_ERROR;
}

aml_ptr_t def_return()
{
  AML_PRELUDE(RETURN_OP)
  printf("DefReturn(%d) ", get_pointer());
  parse_term_arg();
  return (aml_ptr_t){RETURN_OP, NULL};
}

aml_ptr_t def_signal()
{
  return AML_ERROR;
}

aml_ptr_t def_sleep()
{
  return AML_ERROR;
}

aml_ptr_t def_stall()
{
  return AML_ERROR;
}

aml_ptr_t def_while()
{
  AML_PRELUDE(WHILE_OP)
  printf("WhileStatement(%d) ", get_pointer());
  uint32_t length = parse_pkg_length();
  parse_term_arg();
  aml_node_t* while_node = calloc(1, sizeof(aml_node_t));
  parse_term_list(while_node, length);
  return (aml_ptr_t){WHILE_OP, while_node};
}

aml_ptr_t parse_statement_opcode()
{
  return one_of(14, def_break, def_breakpoint, def_continue, def_fatal,
    def_if_else, def_noop, def_notify, def_release, def_reset, def_return,
    def_signal, def_sleep, def_stall, def_while);
}
