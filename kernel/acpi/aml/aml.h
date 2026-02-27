#ifndef __KERNEL_ACPI_AML_INTERNAL_H__
#define __KERNEL_ACPI_AML_INTERNAL_H__

#include "stdlib.h"
#include <stdint.h>
#define EXT_OP_PREFIX        0x5B
#define ROOT_CHAR            0x5C
#define PREFIX_CHAR          0x5E
#define NULL_NAME            0x00
#define DUAL_NAME_PREFIX     0x2E
#define MULTI_NAME_PREFIX    0x2F
#define BYTE_PREFIX          0x0A
#define WORD_PREFIX          0x0B
#define DWORD_PREFIX         0x0C
#define QWORD_PREFIX         0x0E
#define STRING_PREFIX        0x0D
#define ZERO_OP              0x00
#define ONE_OP               0x01
#define ONES_OP              0xFF
#define BUFFER_OP            0x11
#define ALIAS_OP             0x06
#define NAME_OP              0x08
#define SCOPE_OP             0x10
#define ACQUIRE_OP           0x23
#define ADD_OP               0x72
#define OP_REGION_OP         0x80
#define FIELD_OP             0x81
#define AML_PREFIX_ERROR     0xFA
#define NAME_SEG_PREFIX      0xF9
#define REVISION_OP          0x30
#define BANK_FIELD_OP        0x87
#define CREATE_BITFIELD_OP   0x8D
#define CREATE_BYTEFIELD_OP  0x8C
#define CREATE_DWORDFIELD_OP 0x8A
#define CREATE_FIELD_OP      0x13
#define CREATE_QWORDFIELD_OP 0x8F
#define CREATE_WORDFIELD_OP  0x8B
#define DATA_REGION_OP       0x88
#define EXTERNAL_OP          0x15
#define POWER_RES_OP         0x84
#define THERMAL_ZONE_OP      0x85
#define METHOD_OP            0x14
#define TO_HEX_STRING_OP     0x98
#define DEBUG_OP             0x31
#define TO_BUFFER_OP         0x96
#define SUBTRACT_OP          0x74
#define SIZEOF_OP            0x87
#define STORE_OP             0x70
#define WHILE_OP             0xA2
#define LLESS_OP             0x95

#define LEAD_CHAR_OOB(x) x < 0x41 || (x > 0x5A && x != 0x5F)
#define NAME_CHAR_OOB(x) x < 0x30 || (x > 0x39 && LEAD_CHAR_OOB(x))
#define AML_ERROR        (aml_ptr_t){AML_PREFIX_ERROR, NULL};

#define AML_PRELUDE(x)                    \
  if (next_byte() != x) return AML_ERROR;

#define AML_EXT_PRELUDE(x)                            \
  if (next_byte() != EXT_OP_PREFIX) return AML_ERROR; \
  AML_PRELUDE(x)

uint32_t get_next_dword();
void* parse_buffer_definition();

typedef struct
{
  char lead_char;
  char name_char_1;
  char name_char_2;
  char name_char_3;
} __attribute__((packed)) aml_name_segment_t;

typedef struct
{
  aml_name_segment_t first;
  aml_name_segment_t second;
} aml_dual_name_path_t;

typedef struct
{
  uint8_t length;
  aml_name_segment_t* segments;
} aml_multi_name_path_t;

typedef struct
{
  uint8_t prefix_byte;
  void* __ptr;
} __attribute__((packed)) aml_ptr_t;

typedef struct AmlNode
{
  aml_ptr_t data;
  struct AmlNode* parent;
} aml_node_t;

typedef struct
{
  aml_ptr_t source;
  aml_ptr_t alias;
} aml_alias_t;

void aml_parser_init(void*);
void aml_parser_run(void);
typedef aml_ptr_t (*aml_parser_fn)(void);
#endif
