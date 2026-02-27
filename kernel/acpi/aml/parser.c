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
  putchar('\n');

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

    aml_parser_fn parser   = va_arg(parsers, aml_parser_fn);
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
  if (pointer >= MAX_BLOCKS)
  {
    printf("No More Definition Blocks ");
    abort();
  }
  return TABLE->definition_blocks[pointer++];
}
void decrement_pointer()
{
  pointer--;
}

aml_ptr_t parse_data_object()
{
  return one_of(3, computational_data, def_package, def_var_package);
}

aml_ptr_t parse_misc_obj()
{
  uint8_t token = next_byte();
  if (token >= 0x60 && token <= 0x6E) return (aml_ptr_t){token, NULL};
  if (token != EXT_OP_PREFIX) return AML_ERROR;
  return (aml_ptr_t){DEBUG_OP, NULL};
}

aml_ptr_t parse_term_arg()
{
  return one_of(3, parse_expression_opcode, parse_data_object, parse_misc_obj);
}

aml_ptr_t parse_term_obj()
{
  if (pointer >= MAX_BLOCKS) return AML_ERROR;
  return one_of(4, parse_namespace_modifier_obj, parse_named_obj,
    parse_statement_opcode, parse_expression_opcode);
}

void print_term_arg(aml_ptr_t evaluated_term)
{
  if (evaluated_term.prefix_byte >= 0x60 && evaluated_term.prefix_byte <= 0x6E)
  {
    printf("%d", evaluated_term.prefix_byte);
    return;
  }
  switch (evaluated_term.prefix_byte)
  {
    case ONE_OP:
      {
        putchar('1');
        break;
      }
    case ZERO_OP:
      {
        putchar('0');
        break;
      }
    case ONES_OP:
      {
        printf("255");
        break;
      }
    case BYTE_PREFIX:
      {
        printf("%d", *(uint8_t*)evaluated_term.__ptr);
        break;
      }
    case WORD_PREFIX:
      {
        printf("%d", *(uint16_t*)evaluated_term.__ptr);
        break;
      }
    default:
      {
        printf("Unknown");
      }
  }
}

aml_ptr_t evaluate_term_arg(aml_ptr_t term_arg)
{
  switch (term_arg.prefix_byte)
  {
    case ONE_OP:
    case WORD_PREFIX:
      {
        return term_arg; // most of the computational_data terms can be returned
                         // directly since no evaluation is necessary
      }
  }
  if (term_arg.prefix_byte >= 0x60 && term_arg.prefix_byte <= 0x6E)
  {
    return term_arg;
  }
  printf("Error evaluating term %x\n", term_arg.prefix_byte);
  abort();
}

void parse_term_list(aml_node_t* parent, int count)
{
  aml_ptr_t status     = (aml_ptr_t){0, NULL};
  int starting_pointer = pointer;
  aml_node_t* current  = parent;
  while ((pointer - starting_pointer) < count)
  {
    status = parse_term_obj();
    if (status.prefix_byte == AML_PREFIX_ERROR)
    {
      break;
    }
    aml_node_t* new_node = calloc(1, sizeof(aml_node_t));
    new_node->data       = status;
    new_node->parent     = current;
    current              = new_node;
  }
  print_next_definition_block();
  abort();
}

void aml_parser_run()
{
  printf("Max Bytes: %d\n", MAX_BLOCKS);
  aml_node_t* root = calloc(1, sizeof(aml_node_t));
  parse_term_list(root, MAX_BLOCKS);
}
