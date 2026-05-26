#ifndef __AML_INTERNAL_H__
#define __AML_INTERNAL_H__

#include "hashmap.h"

#include <stdint.h>
// misc prefixes
#define EXT_OP_PREFIX 0x5B
#define EXT_DEBUG_OP  0x31
// name string
/*#define ROOT_CHAR         0x5C
#define PREFIX_CHAR       0x5E
#define NULL_NAME         0x00
#define DUAL_NAME_PREFIX  0x2E
#define MULTI_NAME_PREFIX 0x2F
*/
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
// statements
#define IF_OP   0xA0
#define ELSE_OP 0xA1
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

#define TYPE_METHOD       (1 << 0)
#define TYPE_NAMESPACE    (1 << 1)
#define TYPE_REGION       (1 << 2)
#define TYPE_NAME         (1 << 3)
#define TYPE_FIELD_OFFSET (1 << 4)
#define TYPE_MUTEX        (1 << 5)

#define DATA_STR   (1 << 0)
#define DATA_PKG   (1 << 1)
#define DATA_BUF   (1 << 2)
#define DATA_BYTE  (1 << 3)
#define DATA_SHORT (1 << 4)
#define DATA_INT   (1 << 5)
#define DATA_LONG  (1 << 6)

typedef struct
{
  char     signature[4];
  uint32_t length;
  uint8_t  revision;
  uint8_t  checksum;
  uint8_t  oem_id[6];
  uint64_t oem_table_id;
  uint32_t oem_revision;
  uint32_t creator_id;
  uint32_t creator_revision;
  uint8_t  definition_blocks[];
} __attribute__((packed)) acpi_aml_table_t;

typedef struct aml_namespace
{
  struct aml_namespace* parent;
  char*                 name;
  uint8_t*              code;
  hash_map_t*           children;
  hash_map_t*           namespaces;
} aml_namespace_t;

#define MAX_CHARS 1022

typedef struct
{
  int  count;
  char inner[MAX_CHARS];
} aml_name_t;

typedef struct
{
  aml_name_t label;
  uint8_t    data_type;
  void*      data;
} aml_variable_t;

typedef struct
{
  aml_name_t name;
  uint8_t    sync_flags;
} aml_mutex_t;

typedef struct
{
  size_t   size;
  uint8_t* buffer;
} aml_buffer_t;

typedef struct
{
  aml_name_t name;
  uint8_t    flags;
  size_t     len;
  uint8_t*   code;
} aml_method_t;

enum VariableType
{
  VariableUnimplemented = 0,
  VariableInt           = 1,
  VariableStr           = 2,
};

enum AccessType
{
  AnyAcc    = 0,
  ByteAcc   = 1,
  WordAcc   = 2,
  DWordAcc  = 3,
  QWordAcc  = 4,
  BufferAcc = 5
};

enum UpdateRule
{
  Preserve     = 0,
  WriteAsOnes  = 1,
  WriteAsZeros = 2
};

enum FieldUnitType
{
  Reserved       = 0x00,
  Access         = 0x01,
  Connect        = 0x02,
  ExtendedAccess = 0x03,
  Named          = 0xFF,
};

enum AttributeModifier
{
  AttribNormal          = 0,
  AttribBytes           = 1,
  AttribRawBytes        = 2,
  AttribRawProcessBytes = 3,
};

enum AccessAttrib
{
  AttribQuick            = 0x02,
  AttribSendRecv         = 0x04,
  AttribByte             = 0x06,
  AttribWord             = 0x08,
  AttribBlock            = 0x0A,
  AttribProcessCall      = 0x0C,
  AttribBlockProcessCall = 0x0D
};

enum ExtAccessModifier
{
  ExtAttribBytes      = 0x0B,
  ExtAttribRawBytes   = 0x0E,
  ExtAttribRawProcess = 0x0F
};

typedef struct
{
  aml_name_t name;
  uint8_t    region_space;
  size_t     offset;
  size_t     len;
} aml_operation_region_t;

typedef struct
{
  enum AccessType         access_type;
  bool                    should_lock;
  enum UpdateRule         update_rule;
  aml_operation_region_t* region;
} aml_field_t;

typedef struct
{
  int          offset;
  int          len;
  aml_name_t   name;
  aml_field_t* parent;
} aml_named_field_t;

typedef struct
{
  uint8_t type;
  void*   data;
} aml_ptr_t;

void parse_table(acpi_aml_table_t* table);

#endif
