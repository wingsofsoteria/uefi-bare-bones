// clang-format Language: C
#ifndef __KERNEL_CPU_GDT_H__
#define __KERNEL_CPU_GDT_H__

#include <types.h>
typedef struct
{
  uint16_t limit_low;
  uint16_t base_low;
  uint8_t base_mid;
  uint8_t access;
  uint8_t flags_and_limit_high;
  uint8_t base_high;
} __attribute__((packed)) gdt_entry_t;

typedef struct
{
  gdt_entry_t null;
  gdt_entry_t kernel_code;
  gdt_entry_t kernel_data;
  gdt_entry_t user_data;
  gdt_entry_t user_code;
} __attribute__((packed)) gdt_t;

typedef struct
{
  uint16_t size;
  uint64_t offset;
} __attribute__((packed)) gdt_ptr_t;

extern void set_gdt(gdt_ptr_t*);

void load_gdt();

#endif
