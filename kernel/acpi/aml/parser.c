#include "acpi/tables.h"
#include "aml.h"
#include "graphics/pixel.h"
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
    if (return_value.prefix_byte == ERR_PREFIX) continue;
    if (return_value.prefix_byte == ERR_PARSE)
    {
      pointer = current_pointer;
      printf("Parser Error ");
      print_next_definition_block();
      halt_cpu;
    }
    va_end(parsers);
    return return_value;
  }
  pointer = current_pointer;
  va_end(parsers);
  return AML_PREFIX_ERROR;
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
aml_ptr_t parse_data_object()
{
  return one_of(3, computational_data, def_package, def_var_package);
}

aml_ptr_t parse_misc_obj()
{
  uint8_t token = next_byte();
  if (token >= 0x60 && token <= 0x6E)
  {
    printf("MiscObj ");
    return (aml_ptr_t){token, NULL};
  }
  if (token != EXT_OP_PREFIX) return AML_PREFIX_ERROR;
  printf("DebugObj ");
  return (aml_ptr_t){DEBUG_OP, NULL};
}

aml_ptr_t parse_term_arg()
{
  return one_of(3, parse_expression_opcode, parse_data_object, parse_misc_obj);
}

aml_ptr_t parse_term_obj()
{
  if (pointer >= MAX_BLOCKS) return AML_PARSE_ERROR;
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
/*
void write_to_target(aml_ptr_t target, aml_ptr_t value)
{
  if (target.prefix_byte >= 0x60 && target.prefix_byte <= 0x67)
  {
    LOCAL_OBJS[target.prefix_byte - 0x60] = value;
  }
  else if (target.prefix_byte >= 0x68 && target.prefix_byte <= 0x6E)
  {
    ARG_OBJS[target.prefix_byte - 0x68] = value;
  }
  else
  {
    abort();
  }
}

aml_ptr_t read_from_target(aml_ptr_t target)
{
  if (target.prefix_byte >= 0x60 && target.prefix_byte <= 0x67)
  {
    return LOCAL_OBJS[target.prefix_byte - 0x60];
  }
  else if (target.prefix_byte >= 0x68 && target.prefix_byte <= 0x6E)
  {
    return ARG_OBJS[target.prefix_byte - 0x68];
  }
  return AML_ERROR;
}
*/
aml_ptr_t evaluate_term_arg(aml_ptr_t term_arg)
{
  switch (term_arg.prefix_byte)
  {
    case ONE_OP:
    case BYTE_PREFIX:
    case WORD_PREFIX:
      {
        return term_arg; // most of the computational_data terms can be returned
                         // directly since no evaluation is necessary
      }
  }
  if (term_arg.prefix_byte >= 0x60 && term_arg.prefix_byte <= 0x6E)
  {
    return AML_PARSE_ERROR;
    aml_ptr_t object = read_from_target(term_arg);
    if (object.prefix_byte == ERR_PREFIX)
    {
      return term_arg;
    }
    else
    {
      return object;
    }
  }
  return (aml_ptr_t){0xCE, NULL};
}

void parse_term_list(int count)
{
  aml_ptr_t status     = (aml_ptr_t){0, NULL};
  int starting_pointer = pointer;
  while ((pointer - starting_pointer) < count)
  {
    status = parse_term_obj();
    if (status.prefix_byte == ERR_PREFIX)
    {
      break;
    }
    if (status.prefix_byte == ERR_PARSE)
    {
      printf("Parse Error %d\n", starting_pointer);
      abort();
    }
  }
  return;
}
int get_pointer()
{
  return pointer;
}
void move_pointer(int amount_to_move)
{
  pointer += amount_to_move;
}
void aml_parser_run()
{
  clear_screen();
  parse_term_list(MAX_BLOCKS);
  print_next_definition_block();
}
