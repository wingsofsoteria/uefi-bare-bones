#ifndef __KERNEL_ACPI_AML_PARSER_H__
#define __KERNEL_ACPI_AML_PARSER_H__

#include "aml.h"
#include <stdint.h>
uint8_t next_byte();
aml_ptr_t one_of(int, ...);

void parse_term_list(int);
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
aml_ptr_t parse_name_seg();
aml_ptr_t parse_data_object();
aml_ptr_t parse_target();
aml_ptr_t parse_misc_obj();
aml_ptr_t parse_super_name();
aml_ptr_t reference_type_opcode();
aml_ptr_t parse_statement_opcode();
aml_ptr_t evaluate_term_arg(aml_ptr_t);
aml_ptr_t read_from_target(aml_ptr_t);
void write_to_target(aml_ptr_t target, aml_ptr_t value);
void print_term_arg(aml_ptr_t evaluated_term);
void print_name_string(aml_ptr_t);
void print_next_definition_block();
uint32_t parse_pkg_length();
int get_pointer();
void move_pointer(int);
#endif
