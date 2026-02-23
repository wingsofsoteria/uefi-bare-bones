#include "acpi.h"
#include "acpi/tables.h"
#include "aml.h"
#include "graphics/pixel.h"
#include "keyboard.h"
#include "stdlib.h"
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

acpi_dsdt_t* DSDT = NULL;
int MAX_BLOCKS    = 0;

int pointer = 0;

// TODO actually print each definition block

void print_next_definition_block()
{
  clear_screen();
  printf("pointer %d:", pointer);
  for (int i = 0; i < 20; i++)
  {
    if (pointer > MAX_BLOCKS)
    {
      break;
    }
    printf("%x ", DSDT->definition_blocks[pointer++]);
  }
  printf("\n");
}

void loop_print_definition_blocks()
{
  // Print N blocks
  // wait until specific character is pressed - I'll use N for 'next block' and
  // S for 'stop' to keep things simple
  int stopped = 0;
  while (!stopped)
  {
    if (naive_key_released('N'))
    {
      print_next_definition_block(); // clear screen and print
    }
    stopped = is_key_pressed('S') || pointer > MAX_BLOCKS;
  }
}

void dsdt_init()
{
  acpi_fadt_t* fadt  = (acpi_fadt_t*)acpi_get_table("FACP");
  void* dsdt_address = (void*)fadt->x_dsdt;
  if (fadt->x_dsdt == 0)
  {
    dsdt_address = (void*)fadt->dsdt;
  }

  DSDT       = (acpi_dsdt_t*)dsdt_address;
  MAX_BLOCKS = DSDT->header.length - sizeof(acpi_sdt_header_t);
  pointer    = 0;
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
  return DSDT->definition_blocks[pointer++];
}

// returns aml_ptr UNUSED, name_segments(1)
aml_ptr_t parse_name_seg()
{
  uint8_t lead_char       = next_byte();
  uint8_t first_namechar  = next_byte();
  uint8_t second_namechar = next_byte();
  uint8_t third_namechar  = next_byte();
  if (LEAD_CHAR_OOB(lead_char) || NAME_CHAR_OOB(first_namechar) ||
    NAME_CHAR_OOB(second_namechar) || NAME_CHAR_OOB(third_namechar))
    return AML_ERROR;
  aml_name_segment_t* name_segment_ptr = calloc(1, sizeof(aml_name_segment_t));
  aml_name_segment_t name_segment      = (aml_name_segment_t){
    lead_char, first_namechar, second_namechar, third_namechar};
  *name_segment_ptr = name_segment;
  return (aml_ptr_t){
    NULL_NAME, name_segment_ptr}; // we don't care about the prefix unless its
                                  // AML_PREFIX_ERROR
}

// returns aml_ptr DUAL_NAME_PREFIX, name_segments(2)
aml_ptr_t parse_dual_name_path()
{
  uint8_t token = next_byte();
  if (token != DUAL_NAME_PREFIX) return AML_ERROR;
  aml_ptr_t first_seg = parse_name_seg();
  if (first_seg.prefix_byte == AML_PREFIX_ERROR) return AML_ERROR;
  aml_ptr_t second_seg = parse_name_seg();
  if (second_seg.prefix_byte == AML_PREFIX_ERROR) return AML_ERROR;

  aml_name_segment_t* name_path = calloc(2, sizeof(aml_name_segment_t));
  name_path[0] =
    *(aml_name_segment_t*)
       first_seg.__ptr; // TODO same possible errors as multi_name_path
  name_path[1] = *(aml_name_segment_t*)second_seg.__ptr;
  free(first_seg.__ptr);
  free(second_seg.__ptr);
  return (aml_ptr_t){DUAL_NAME_PREFIX, name_path};
}

// returns aml_ptr MULTI_NAME_PREFIX, name_segment(n) where n is an arbitrary
// number of name segments
aml_ptr_t parse_multi_name_path()
{
  uint8_t token = next_byte();
  if (token != MULTI_NAME_PREFIX) return AML_ERROR;
  uint8_t num_name_segs = next_byte();
  aml_name_segment_t* name_path =
    calloc(num_name_segs, sizeof(aml_name_segment_t));
  for (int i = 0; i < num_name_segs; i++)
  {
    aml_ptr_t name_segment = parse_name_seg();
    if (name_segment.prefix_byte == AML_PREFIX_ERROR) return AML_ERROR;
    aml_name_segment_t* name_seg_ptr = name_segment.__ptr;
    name_path[i] = *name_seg_ptr; // TODO possible error, might need to memcpy
    free(name_seg_ptr);           // TODO unlikely error based on previous TODO
  }

  return (aml_ptr_t){MULTI_NAME_PREFIX, name_path};
}

aml_ptr_t parse_null_name()
{
  uint8_t token = next_byte();
  if (token != NULL_NAME) return AML_ERROR;
  return (aml_ptr_t){NULL_NAME, NULL};
}

aml_ptr_t parse_name_path()
{
  return one_of(4, parse_null_name, parse_multi_name_path, parse_dual_name_path,
    parse_name_seg);
}

aml_ptr_t parse_name_string()
{
  uint8_t token = next_byte();
  if (token == ROOT_CHAR)
  {
    aml_ptr_t name_path = parse_name_path();
    return (aml_ptr_t){ROOT_CHAR, name_path.__ptr};
  }
  else
  {
    while (token == PREFIX_CHAR)
    {
      token = next_byte();
    }

    aml_ptr_t name_path = parse_name_path();
    return (aml_ptr_t){PREFIX_CHAR, name_path.__ptr};
  }
}

aml_ptr_t parse_def_alias()
{
  uint8_t token = next_byte();
  if (token != ALIAS_OP) return AML_ERROR;
  aml_ptr_t source       = parse_name_string();
  aml_ptr_t alias        = parse_name_string();
  aml_alias_t* def_alias = calloc(1, sizeof(aml_alias_t));
  def_alias->source      = source;
  def_alias->alias       = alias;
  return (aml_ptr_t){ALIAS_OP, def_alias};
}

aml_ptr_t parse_def_name()
{
  return AML_ERROR;
}

aml_ptr_t parse_def_scope()
{
  return AML_ERROR;
}

aml_ptr_t parse_namespace_modifier_obj()
{
  return one_of(3, parse_def_alias, parse_def_name, parse_def_scope);
}

aml_ptr_t parse_named_obj()
{
  return AML_ERROR;
}

aml_ptr_t parse_statement_opcode()
{
  return AML_ERROR;
}

aml_ptr_t parse_expression_opcode()
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

  while (status.prefix_byte != AML_PREFIX_ERROR)
  {
    status = parse_term_obj();
  }
}

void dsdt_parse()
{
  printf("DSDT\n");
  parse_term_list();
}
