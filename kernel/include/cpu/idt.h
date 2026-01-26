// clang-format Language: C
#ifndef __KERNEL_CPU_IDT_H__
#define __KERNEL_CPU_IDT_H__

#include <stdint.h>

typedef struct
{
  uint64_t r15, r14, r13, r12, r11, r10, r9, r8, rbp, rdi, rsi, rdx, rcx, rbx, rax, isr, err, rip, cs, rflags;
} __attribute__((packed)) isr_stack_t;

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

extern void* isr_stub_table[256];

void load_idt();

extern volatile uint64_t ticks;
#endif
