#include "acpi.h"
#include "acpi/tables.h"
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
  if (first_byte == 0x5C || first_byte == 0x5E)
  {
    prefix_byte = first_byte;
    first_byte  = get_next_byte();
  }
  if (first_byte == 0x00)
  {
    char* name_segment = calloc(2, sizeof(char));
    name_segment[0]    = prefix_byte;
    name_segment[1]    = 0x00;
    return name_segment;
  }
  if (first_byte == 0x2E)
  {
    num_segments = 2;
  }
  if (first_byte == 0x2F)
  {
    num_segments = get_next_byte();
  }
  return parse_name_path(num_segments, prefix_byte);
}

void parse_term_list()
{
}

void parse_scope_definition()
{
  uint32_t pkg_length = parse_pkg_length();
  char* name_string   = parse_name_string();
  printf("SCOPE DEFINITION: %d, %s\n", pkg_length, name_string);
}

void dsdt_parse()
{
  printf("DSDT\n");
  char next_byte = get_next_byte();
  switch (next_byte)
  {
    case 0x10:
      {
        parse_scope_definition();
        break;
      }
  }
}
