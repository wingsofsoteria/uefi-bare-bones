#include "acpi.h"
#include "acpi/tables.h"
#include "aml.h"
#include "graphics/pixel.h"
#include "keyboard.h"
#include "stdlib.h"
#include <stdio.h>

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

char get_next_byte()
{
  return DSDT->definition_blocks[pointer++];
}

char peek_next_byte(int peek_point)
{
  return DSDT->definition_blocks[pointer + peek_point];
}

uint32_t parse_pkg_length()
{
  char pkg_lead_byte    = get_next_byte();
  char byte_data_count  = pkg_lead_byte >> 6;
  char pkg_length_upper = (pkg_lead_byte >> 4) & 0x03;
  char pkg_length_lower = (pkg_lead_byte & 0x0F);
  uint32_t pkg_length   = (pkg_length_upper << 4) | pkg_length_lower;
  while (byte_data_count > 0)
  {
    char next_lsb = get_next_byte();
    pkg_length    = (pkg_length << 8) | next_lsb;
    byte_data_count--;
  }
  return pkg_length;
}

char* parse_name_path(int num_segments, char prefix)
{
  int size        = num_segments * 4;
  int start_point = 0;

  if (prefix)
  {
    size++;
    start_point = 1;
  }
  char* name_segment = calloc(size, sizeof(char));
  name_segment[0]    = prefix;
  for (int i = 0; i < num_segments; i++)
  {
    name_segment[start_point]      = get_next_byte();
    name_segment[start_point + 1]  = get_next_byte();
    name_segment[start_point + 2]  = get_next_byte();
    name_segment[start_point + 3]  = get_next_byte();
    start_point                   += 4;
  }
  return name_segment;
}

char* parse_name_string()
{
  char first_byte  = get_next_byte();
  char prefix_byte = 0;
  int num_segments = 1;
  if (first_byte == ROOT_CHAR || first_byte == PREFIX_CHAR)
  {
    prefix_byte = first_byte;
    first_byte  = get_next_byte();
  }
  if (first_byte == NULL_NAME)
  {
    char* name_segment = calloc(2, sizeof(char));
    name_segment[0]    = prefix_byte;
    name_segment[1]    = NULL_NAME;
    return name_segment;
  }
  if (first_byte == DUAL_NAME_PREFIX)
  {
    num_segments = 2;
  }
  if (first_byte == MULTI_NAME_PREFIX)
  {
    num_segments = get_next_byte();
  }
  return parse_name_path(num_segments, prefix_byte);
}

void parse_scope_definition()
{
  uint32_t pkg_length = parse_pkg_length();
  char* name_string   = parse_name_string();
  printf("SCOPE DEFINITION: %d, %s\n", pkg_length, name_string);
}

void parse_alias_definition()
{
  char* first_string  = parse_name_string();
  char* second_string = parse_name_string();
  printf("ALIAS DEFINITION: %s == %s\n", first_string, second_string);
}

uint16_t get_next_word()
{
  char lsb_data = get_next_byte();
  char msb_data = get_next_byte();
  return ((uint16_t)msb_data << 8) | lsb_data;
}

uint32_t get_next_dword()
{
  uint16_t lsb_data = get_next_word();
  uint16_t msb_data = get_next_word();
  return ((uint32_t)msb_data << 16) | lsb_data;
}

uint64_t get_next_qword()
{
  uint32_t lsb_data = get_next_dword();
  uint32_t msb_data = get_next_dword();
  return ((uint64_t)msb_data << 32) | lsb_data;
}

char* parse_string()
{
  char next_char     = get_next_byte();
  int expected_size  = 16;
  int counter        = 0;
  char* ascii_string = calloc(expected_size, sizeof(char));
  while (next_char)
  {
    ascii_string[counter++] = next_char;
    if (counter >= expected_size - 1)
    {
      expected_size *= 2;
      ascii_string   = realloc(ascii_string, expected_size * sizeof(char));
    }
    next_char = get_next_byte();
  }
  ascii_string[counter] = next_char;
  return ascii_string;
}

void* parse_computational_data()
{
  char current_byte = peek_next_byte(-1);
  switch (current_byte)
  {
    case BYTE_PREFIX:
      {
        char byte_data = get_next_byte();
        char* byte_ptr = calloc(1, sizeof(char));
        *byte_ptr      = byte_data;
        return byte_ptr;
      }
    case WORD_PREFIX:
      {
        uint16_t word_data = get_next_word();
        uint16_t* word_ptr = calloc(1, sizeof(uint16_t));
        *word_ptr          = word_data;
        return word_ptr;
      }
    case DWORD_PREFIX:
      {
        uint32_t dword_data = get_next_dword();
        uint32_t* dword_ptr = calloc(1, sizeof(uint32_t));
        *dword_ptr          = dword_data;
        return dword_ptr;
      }
    case QWORD_PREFIX:
      {
        uint64_t qword_data = get_next_qword();
        uint64_t* qword_ptr = calloc(1, sizeof(uint64_t));
        *qword_ptr          = qword_data;
        return qword_ptr;
      }
  }
  char* const_obj = calloc(1, sizeof(char));
  *const_obj      = current_byte;
  return const_obj;
}

void parse_add_definition()
{
}

/*
 * based on my understanding of AML so far
 * a TermArg is an expression that evaluates to a value
 * so eventually parse_term_arg should be returning a function that returns a
 * void pointer that represents some other type determined by context
 */

void parse_term_arg()
{
  uint8_t next_byte = get_next_byte();
  switch (next_byte)
  {
    case EXT_OP_PREFIX:
      {
        uint8_t extended_byte = get_next_byte();
        switch (extended_byte)
        {
          case ACQUIRE_OP:
            {
              // return parse_acquire_definition();
              abort();
            }
        }
      }
    case ADD_OP:
      {
        parse_add_definition();
        break;
      }
  }
}

void* parse_buffer_definition()
{
  uint32_t pkg_length = parse_pkg_length();

  return NULL;
}

void* parse_data_ref_object()
{
  uint8_t next_byte = get_next_byte();
  switch (next_byte)
  {
    case BYTE_PREFIX:
    case WORD_PREFIX:
    case DWORD_PREFIX:
    case QWORD_PREFIX:
    case ZERO_OP:
    case ONE_OP:
    case ONES_OP:
      {
        return parse_computational_data();
      }
    case STRING_PREFIX:
      {
        return parse_string();
      }
    case EXT_OP_PREFIX:
      {
        char revision_op   = get_next_byte();
        char* revision_ptr = calloc(1, sizeof(char));
        *revision_ptr      = revision_op;
        return revision_ptr;
      }
    case BUFFER_OP:
      {
        return parse_buffer_definition();
      }
  }

  return NULL;
}

void parse_name_definition()
{
  char* string = parse_name_string();
  parse_data_ref_object();
}

void parse_term_list()
{
  char next_byte = get_next_byte();

  switch (next_byte)
  {
    case ALIAS_OP:
      {
        parse_alias_definition();
        break;
      }
    case NAME_OP:
      {
        parse_name_definition();
        break;
      }
    case SCOPE_OP:
      {
        parse_scope_definition();
        break;
      }
  }
}

void dsdt_parse()
{
  printf("DSDT\n");
  char next_byte = get_next_byte();
  switch (next_byte)
  {
    case SCOPE_OP:
      {
        parse_scope_definition();
        break;
      }
  }
}
