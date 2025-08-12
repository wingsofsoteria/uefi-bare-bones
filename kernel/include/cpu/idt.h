#ifndef __KERNEL_CPU_IDT_H__
#define __KERNEL_CPU_IDT_H__

#include <stdint.h>
extern void* isr_stub_table[256];

typedef struct
{
  uint16_t size;
  uint64_t offset;
} __attribute__((packed)) idt_ptr_t;

typedef struct
{
  uint16_t offset_low;
  uint16_t segment_selector;
  uint8_t ist;
  uint8_t flags;
  uint16_t offset_mid;
  uint32_t offset_high;
  uint32_t reserved_high;
} __attribute__((packed)) idt_entry_t;

typedef struct
{
  idt_entry_t entries[256];
} __attribute__((packed)) idt_t;

extern void set_idt(idt_ptr_t*);
void load_idt();

#endif
