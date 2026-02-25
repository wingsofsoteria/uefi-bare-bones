#ifndef __KERNEL_ACPI_AML_PARSER_H__
#define __KERNEL_ACPI_AML_PARSER_H__

#include "aml.h"
#include <stdint.h>
uint8_t next_byte();
aml_ptr_t one_of(int, ...);
void decrement_pointer();

aml_ptr_t parse_name_string();
aml_ptr_t parse_namespace_modifier_obj();
aml_ptr_t parse_named_obj();
aml_ptr_t parse_data_ref_object();
aml_ptr_t parse_term_arg();
aml_ptr_t parse_expression_opcode();
aml_ptr_t def_package();
aml_ptr_t def_var_package();
aml_ptr_t def_buffer();
aml_ptr_t computational_data();

void print_next_definition_block();
uint32_t parse_pkg_length();
#endif
