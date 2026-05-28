#ifndef __AML_INTERNAL_H__
#define __AML_INTERNAL_H__

// misc prefixes
#include "types.h"
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

void parse_table(acpi_aml_table_t* table);
#endif
