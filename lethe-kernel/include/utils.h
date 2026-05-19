#pragma once

#include "assert.h"
#include "config.h"

#include <stdint.h>
#ifdef QEMU_DEBUG
  #define SERIAL_PORT 0xE9
#endif
#ifdef VBOX_DEBUG
  #define SERIAL_PORT 0x3F8
#endif

__attribute__((__noreturn__)) static inline void halt()
{
  asm("cli");
  for (;;) { asm volatile("hlt"); }
  __builtin_unreachable();
}

#define panic(msg) __panic(msg, __source);

__attribute__((__noreturn__)) void __panic(char* msg, struct source_location);

inline static uint64_t read_cr3()
{
  uint64_t cr3;
  asm volatile("mov %%cr3, %0"
    : "=r"(cr3));
  return (cr3 >> 12) << 12;
}

inline static uint64_t read_cr2()
{
  uint64_t cr2;
  asm volatile("mov %%cr2, %0"
    : "=r"(cr2));
  return cr2;
}

inline static void flush_tlb(uint64_t page)
{
  asm volatile("invlpg (%0)" ::"b"(page)
    : "memory");
}
// NOLINTNEXTLINE(*-const-parameter)
static inline void read_msr(uint32_t msr, uint32_t* low, uint32_t* high)
{
  asm volatile("rdmsr"
    : "=a"(*low), "=d"(*high)
    : "c"(msr));
}

inline static void write_msr(uint32_t msr, uint32_t low, uint32_t high)
{
  asm volatile("wrmsr"
    :
    : "a"(low), "d"(high), "c"(msr));
}

inline static void outb(uint16_t port, uint8_t val)
{
  asm volatile("outb %b0, %1"
    :
    : "a"(val), "Nd"(port)
    : "memory");
}

inline static uint8_t inb(uint16_t port)
{
  uint8_t val;
  asm volatile("inb %1, %b0"
    : "=a"(val)
    : "Nd"(port)
    : "memory");
  return val;
}

inline static uint16_t inw(uint16_t port)
{
  uint16_t val;
  asm volatile("inw %1, %w0"
    : "=a"(val)
    : "Nd"(port)
    : "memory");
  return val;
}

inline static uint32_t ind(uint16_t port)
{
  uint32_t val;
  asm volatile("inl %1, %k0"
    : "=a"(val)
    : "Nd"(port)
    : "memory");
  return val;
}

inline static uint64_t rdtsc()
{
  uint64_t low;
  uint64_t high;
  asm volatile("rdtsc"
    : "=a"(low), "=d"(high));
  return ((high << 32) | low);
}

void     abort();
void     walk_stack();
char*    resolve_function_name(uint64_t);
uint64_t resolve_function_address(char*);
