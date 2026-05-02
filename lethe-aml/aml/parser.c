#include "parser.h"

#include "aml.h"
#include "host.h"
#include "stdlib.h"

#include <stdint.h>
static int MAX_BLOCKS = 0;

static int               pointer            = 0;
int                      __current_anchor__ = 0;
static acpi_aml_table_t* TABLE              = NULL;

void aml_parser_init(void* address)
{
  TABLE      = (acpi_aml_table_t*)address;
  MAX_BLOCKS = TABLE->length - 36;
  pointer    = 0;
}

void print_next_definition_block()
{
  int current_pointer = pointer;
  AML_LOG("%d: ", current_pointer);
  for (int i = 0; i < 20; i++)
    {
      if (pointer >= MAX_BLOCKS) { break; }
      AML_LOG("%x ", TABLE->definition_blocks[pointer++]);
    }
  AML_LOG("\n");

  pointer = current_pointer;
}

/*aml_ptr_t one_of(void* map_ptr, int parser_count, ...)
{
  va_list parsers;
  va_start(parsers, parser_count);
  int current_pointer = pointer;
  for (int i = 0; i < parser_count; i++)
  {
    pointer = current_pointer;

    aml_parser_fn parser   = va_arg(parsers, aml_parser_fn);
    aml_ptr_t return_value = parser(map_ptr);
    if (return_value.prefix_byte == ERR_PREFIX)
    {
      continue;
    }
    if (return_value.prefix_byte == ERR_PARSE)
    {
      va_end(parsers);
      return AML_PARSE_ERROR;
    }
    va_end(parsers);
    return return_value;
  }
  pointer = current_pointer;
  va_end(parsers);
  return AML_PREFIX_ERROR;
}*/

uint8_t peek_byte(int peek_location)
{
  if (pointer >= MAX_BLOCKS) { AML_EXIT(); }
  return TABLE->definition_blocks[pointer + peek_location];
}

uint8_t next_byte()
{
  if (pointer >= MAX_BLOCKS) { AML_EXIT(); }
  return TABLE->definition_blocks[pointer++];
}

void* table_ptr() { return (void*)(TABLE->definition_blocks + pointer); }

aml_ptr_t parse_data_object()
{
  aml_ptr_t status;
  TRY_PARSE(computational_data);
  TRY_PARSE(def_package);
  TRY_PARSE(def_var_package);
  return AML_PREFIX_ERROR;
}

aml_ptr_t parse_misc_obj()
{
  uint8_t token = next_byte();
  if (token >= 0x60 && token <= 0x6E) { return (aml_ptr_t){ token, NULL }; }
  if (token != EXT_OP_PREFIX) { return AML_PREFIX_ERROR; }
  return (aml_ptr_t){ EXT_DEBUG_OP, NULL };
}

aml_ptr_t parse_term_arg()
{
  aml_ptr_t status;
  TRY_PARSE(parse_data_object);
  TRY_PARSE(parse_misc_obj);
  TRY_PARSE(parse_expression_opcode);
  return AML_PREFIX_ERROR;
}

static aml_ptr_t parse_term_obj(void* map_ptr)
{
  if (pointer >= MAX_BLOCKS) { return AML_PARSE_ERROR; }
  aml_ptr_t status;
  TRY_PARSE(parse_namespace_modifier_obj, map_ptr);
  TRY_PARSE(parse_named_obj, NULL);
  TRY_PARSE(parse_statement_opcode);
  TRY_PARSE(parse_expression_opcode);
  return AML_PREFIX_ERROR;
}

aml_ptr_t parse_term_list(void* map_ptr, int count)
{
  int       old_pointer = pointer;
  aml_ptr_t status;
  // aml_node_t* static_list_root = aml_create_node();
  // aml_node_t* list_root        = static_list_root;
  while (1)
    {
      AML_SET_ANCHOR;
      status          = parse_term_obj(map_ptr);
      int new_pointer = pointer;
      if (status.prefix_byte == ERR_PREFIX) { return AML_PREFIX_ERROR; }
      if ((new_pointer - old_pointer) > count) { AML_EXIT(); }
      if ((new_pointer - old_pointer) == count) { break; }
      if (status.prefix_byte == ERR_PARSE) { AML_EXIT(); }

      // aml_node_t* list_child = aml_create_node();
      // list_child->data       = status;
      // aml_append_node(list_root, list_child);
      // list_root = list_child;
    }
  return (aml_ptr_t){ TERM_LIST_PREFIX, NULL };
}

int table_length() { return MAX_BLOCKS; }

int get_pointer() { return pointer; }

void move_pointer(int amount_to_move) { pointer += amount_to_move; }

void set_pointer(int new_pointer) { pointer = new_pointer; }

void aml_parser_run()
{
  init_map();
  aml_ptr_t node_ptr = parse_term_list(root(), MAX_BLOCKS);
  print_next_definition_block();
}
