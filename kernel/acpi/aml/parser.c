#include "acpi/tables.h"
#include "aml.h"
#include "stdlib.h"
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include "parser.h"
int MAX_BLOCKS = 0;

int pointer = 0;

acpi_aml_table_t* TABLE = NULL;

void aml_parser_init(void* address)
{
  TABLE      = (acpi_aml_table_t*)address;
  MAX_BLOCKS = TABLE->header.length - sizeof(acpi_sdt_header_t);
  pointer    = 0;
}

void print_next_definition_block()
{
  int current_pointer = pointer;
  printf("pointer %d:", pointer);
  for (int i = 0; i < 20; i++)
  {
    if (pointer >= MAX_BLOCKS) break;
    printf("%x ", TABLE->definition_blocks[pointer++]);
  }
  printf("\n");

  pointer = current_pointer;
}

aml_ptr_t one_of(int parser_count, ...)
{
  va_list parsers;
  va_start(parsers, parser_count);
  int current_pointer = pointer;
  for (int i = 0; i < parser_count; i++)
  {
    pointer = current_pointer;

    aml_parser_fn parser = va_arg(parsers, aml_parser_fn);

    aml_ptr_t return_value = parser();

    if (return_value.prefix_byte != AML_PREFIX_ERROR)
    {
      va_end(parsers);
      return return_value;
    }
  }
  pointer = current_pointer;
  va_end(parsers);
  return AML_ERROR;
}

uint8_t next_byte()
{
  if (pointer >= MAX_BLOCKS) abort();
  return TABLE->definition_blocks[pointer++];
}
void decrement_pointer()
{
  pointer--;
}
aml_ptr_t parse_statement_opcode()
{
  printf("StatementOp\n");
  return AML_ERROR;
}
aml_ptr_t parse_data_object()
{
  return one_of(3, computational_data, def_package, def_var_package);
}

aml_ptr_t parse_arg_obj()
{
  uint8_t token = next_byte();
  if (token < 0x68 || token > 0x6E) return AML_ERROR;
  return (aml_ptr_t){token, NULL};
}

aml_ptr_t parse_local_obj()
{
  uint8_t token = next_byte();
  if (token < 0x60 || token > 0x67) return AML_ERROR;
  return (aml_ptr_t){token, NULL};
}

aml_ptr_t parse_term_arg()
{
  return one_of(4, parse_expression_opcode, parse_data_object, parse_arg_obj,
    parse_local_obj);
}

aml_ptr_t parse_data_ref_object()
{
  return AML_ERROR;
}

aml_ptr_t parse_term_obj()
{
  if (pointer >= MAX_BLOCKS) return AML_ERROR;
  return one_of(4, parse_namespace_modifier_obj, parse_named_obj,
    parse_statement_opcode, parse_expression_opcode);
}

void parse_term_list()
{
  aml_ptr_t status = (aml_ptr_t){0, NULL};
  printf("TermList\n");
  while (status.prefix_byte != AML_PREFIX_ERROR)
  {
    status = parse_term_obj();
  }
  pointer--;
  print_next_definition_block();
  abort();
}

void aml_parser_run()
{
  parse_term_list();
}
