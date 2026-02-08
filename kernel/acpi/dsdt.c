#include "acpi.h"
#include "graphics/pixel.h"
#include "keyboard.h"
#include <stdio.h>
void print_next_definition_block()
{
  clear_screen();
  printf("TODO DEFINITION BLOCK HERE");
}

void loop_print_definition_blocks()
{
  // Print N blocks
  // wait until specific character is pressed - I'll use N for 'next block' and
  // S for 'stop' to keep things simple
  while (!is_key_pressed('S'))
  {
    if (is_key_pressed('N'))
    {
      print_next_definition_block(); // clear screen and print
    }
    else
    {
      continue; // TODO maybe add sleep here for delay if needed
    }
  }
}

void get_definition_block_count()
{
  acpi_fadt_t* fadt  = (acpi_fadt_t*)acpi_get_table("FACP");
  void* dsdt_address = (void*)fadt->x_dsdt;
  if (fadt->x_dsdt == 0)
  {
    dsdt_address = (void*)fadt->dsdt;
  }

  acpi_dsdt_t* dsdt = (acpi_dsdt_t*)dsdt_address;

  int length = (dsdt->header.length - sizeof(acpi_sdt_header_t));

  printf("Number of definition blocks: %d\n", length);
}
