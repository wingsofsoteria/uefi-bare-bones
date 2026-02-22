#ifndef __KERNEL_ACPI_AML_INTERNAL_H__
#define __KERNEL_ACPI_AML_INTERNAL_H__

#include <stdint.h>
#define EXT_OP_PREFIX     0x5B
#define ROOT_CHAR         0x5C
#define PREFIX_CHAR       0x5E
#define NULL_NAME         0x00
#define DUAL_NAME_PREFIX  0x2E
#define MULTI_NAME_PREFIX 0x2F
#define BYTE_PREFIX       0x0A
#define WORD_PREFIX       0x0B
#define DWORD_PREFIX      0x0C
#define QWORD_PREFIX      0x0E
#define STRING_PREFIX     0x0D
#define ZERO_OP           0x00
#define ONE_OP            0x01
#define ONES_OP           0xFF
#define BUFFER_OP         0x11
#define ALIAS_OP          0x06
#define NAME_OP           0x08
#define SCOPE_OP          0x10
#define ACQUIRE_OP        0x23
#define ADD_OP            0x72
#define OP_REGION_OP      0x80
#define FIELD_OP          0x81

void parse_term_list();
uint32_t get_next_dword();
void* parse_buffer_definition();

typedef struct
{
  char lead_char;
  char name_char_1;
  char name_char_2;
  char name_char_3;
} aml_name_segment_t;

typedef struct
{
  char prefix_byte;
  void* __ptr;
} __attribute__((packed)) aml_cursed_ptr_t;
#endif
