#pragma once

#include "types.h"
void       parse_table(acpi_aml_table_t* table);
aml_ptr_t* get_object(char*);
void       execute_method(aml_method_t*, ...);
void       execute_sleep(int);
