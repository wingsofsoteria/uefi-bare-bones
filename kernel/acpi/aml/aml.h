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
#define ERR_PREFIX           0xFA
#define NAME_SEG_PREFIX      0xF9
#define ERR_PARSE            0xF8
#define TERM_LIST_PREFIX     0xF7
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
#define DEREF_OF_OP          0x83
#define INDEX_OP             0x88
#define INCREMENT_OP         0x75
#define DEVICE_OP            0x82
#define IF_OP                0xA0
#define ELSE_OP              0xA1
#define LEQUAL_OP            0x93
#define RETURN_OP            0xA4
#define PACKAGE_OP           0x12
#define MUTEX_OP             0x01
#define SHIFT_LEFT_OP        0x79
#define RELEASE_OP           0x27
#define OR_OP                0x7D
#define NOT_OP               0x80
#define VAR_PACKAGE_OP       0x13

#define LEAD_CHAR_OOB(x) x < 0x41 || (x > 0x5A && x != 0x5F)
#define NAME_CHAR_OOB(x) x < 0x30 || (x > 0x39 && LEAD_CHAR_OOB(x))
#define AML_ERR_CHECK(x)                                         \
  if (x.prefix_byte == ERR_PREFIX || x.prefix_byte == ERR_PARSE) \
  {                                                              \
    return AML_PARSE_ERROR;                                      \
  }
#define AML_PREFIX_ERROR \
  (aml_ptr_t)            \
  {                      \
    ERR_PREFIX, NULL     \
  }

#define AML_PARSE_ERROR \
  (aml_ptr_t)           \
  {                     \
    ERR_PARSE, NULL     \
  }
#define AML_PRELUDE(x)                          \
  if (next_byte() != x) return AML_PREFIX_ERROR

#define AML_EXT_PRELUDE(x)                                   \
  if (next_byte() != EXT_OP_PREFIX) return AML_PREFIX_ERROR; \
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
  uint8_t name_prefix;
  union
  {
    aml_name_segment_t name_seg;
    aml_dual_name_path_t dual_seg;
    aml_multi_name_path_t multi_seg;
  };

} aml_name_string_t;

typedef struct
{
  uint8_t prefix_byte;
  void* __ptr;
} __attribute__((packed)) aml_ptr_t;

typedef struct AmlNode
{
  aml_ptr_t data;
  char* name;
  struct AmlNode* parent;
  struct AmlNode* child;
} aml_node_t;

typedef struct
{
  aml_ptr_t method_name;
  uint8_t method_flags;
  uint32_t length;
  char* code;
} aml_method_t;

typedef struct
{
  aml_ptr_t source;
  aml_ptr_t alias;
} aml_alias_t;

void aml_parser_init(void*);
void aml_parser_run(void);
aml_node_t* aml_root_node();
void aml_append_node(aml_node_t* parent, aml_node_t* this);
aml_node_t* aml_create_node();
void aml_node_init();
typedef aml_ptr_t (*aml_parser_fn)(void);
#endif
