#ifndef __AML_INTERNAL_H__
#define __AML_INTERNAL_H__

#include "host.h"
// misc prefixes
#define EXT_OP_PREFIX 0x5B
#define EXT_DEBUG_OP  0x31
// name string
#define ROOT_CHAR         0x5C
#define PREFIX_CHAR       0x5E
#define NULL_NAME         0x00
#define DUAL_NAME_PREFIX  0x2E
#define MULTI_NAME_PREFIX 0x2F
// computational data
#define BYTE_PREFIX     0x0A
#define WORD_PREFIX     0x0B
#define DWORD_PREFIX    0x0C
#define QWORD_PREFIX    0x0E
#define STRING_PREFIX   0x0D
#define STRING_OOB      0x7F
#define ZERO_OP         0x00
#define ONE_OP          0x01
#define ONES_OP         0xFF
#define EXT_REVISION_OP 0x30
// expressions
#define EXT_ACQUIRE_OP 0x23
#define ADD_OP         0x72
#define BUFFER_OP      0x11
#define PACKAGE_OP     0x12
// namespace modifiers
#define ALIAS_OP 0x06
#define NAME_OP  0x08
#define SCOPE_OP 0x10
// self defined
#define ERR_PREFIX       0xFA
#define NAME_SEG_PREFIX  0xF9
#define ERR_PARSE        0xF8
#define TERM_LIST_PREFIX 0xF7
// statements
#define IF_OP 0xA0
// named objs
#define EXT_BANK_FIELD_OP    0x87
#define EXT_CREATE_FIELD_OP  0x13
#define EXT_DATA_REGION_OP   0x88
#define EXT_DEVICE_OP        0x82
#define EXT_EVENT_OP         0x02
#define EXT_FIELD_OP         0x81
#define EXT_INDEX_FIELD_OP   0x86
#define EXT_MUTEX_OP         0x01
#define EXT_OP_REGION_OP     0x80
#define EXT_POWER_RES_OP     0x84
#define EXT_THERMAL_ZONE_OP  0x85
#define METHOD_OP            0x14
#define EXTERNAL_OP          0x15
#define CREATE_BITFIELD_OP   0x8D
#define CREATE_BYTEFIELD_OP  0x8C
#define CREATE_DWORDFIELD_OP 0x8A
#define CREATE_QWORDFIELD_OP 0x8F
#define CREATE_WORDFIELD_OP  0x8B

#define LEAD_CHAR_OOB(x) x < 0x41 || (x > 0x5A && x != 0x5F)
#define NAME_CHAR_OOB(x) x < 0x30 || (x > 0x39 && LEAD_CHAR_OOB(x))
#define AML_SET_ANCHOR   __current_anchor__ = get_pointer()
#define AML_ERR_CHECK_ABRT(x)                                         \
  if (x.prefix_byte == ERR_PREFIX || x.prefix_byte == ERR_PARSE) \
  {                                                              \
    set_pointer(__current_anchor__);                             \
    AML_LOG("ERR Check Failed");                                 \
print_next_definition_block();\
AML_EXIT()\
}
#define AML_ERR_CHECK(x)                                         \
if (x.prefix_byte == ERR_PREFIX || x.prefix_byte == ERR_PARSE) \
{                                                              \
set_pointer(__current_anchor__);                             \
AML_LOG("ERR Check Failed");                                 \
return x;\
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
  char signature[4];
  uint32_t length;
  uint8_t revision;
  uint8_t checksum;
  uint8_t oem_id[6];
  uint64_t oem_table_id;
  uint32_t oem_revision;
  uint32_t creator_id;
  uint32_t creator_revision;
  uint8_t definition_blocks[];
} __attribute__((packed)) acpi_aml_table_t;

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
extern int __current_anchor__;
#endif
