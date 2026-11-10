#include <stdio.h>

#include "dsdt.h"
struct acpi_aml_table_t;
typedef struct acpi_aml_table_t aml_table;

void parse_table(aml_table*);

int main()
{
  parse_table((aml_table*)aml_driver_dsdt_dat);
  return 0;
}
