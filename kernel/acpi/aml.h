#ifndef __KERNEL_ACPI_AML_INTERNAL_H__
#define __KERNEL_ACPI_AML_INTERNAL_H__

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
#endif
