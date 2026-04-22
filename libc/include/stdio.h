// clang-format Language: C
#ifndef _STDIO_H
#define _STDIO_H

#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/cdefs.h>

#define EOF (-1)
#ifdef QEMU_DEBUG
  #define SERIAL_PORT 0xE9
#endif
#ifdef VBOX_DEBUG
  #define SERIAL_PORT 0x3F8
#endif
#define abort_msg(format, ...)                   \
  printf("ABORT [%s:%d]: ", __func__, __LINE__); \
  __abort_msg(format, ##__VA_ARGS__);

#define abort()                                \
  printf("ABORT [%s:%d]", __func__, __LINE__); \
  __abort();

int vprintf(const char* restrict format, va_list parameters);
int printf(const char* __restrict, ...);
int putchar(int);
int puts(const char*);

static inline uint64_t read_cr3()
{
  uint64_t cr3;
  asm volatile("mov %%cr3, %0"
    : "=r"(cr3));
  return (cr3 >> 12) << 12;
}
static inline uint64_t read_cr2()
{
  uint64_t cr2;
  asm volatile("mov %%cr2, %0"
    : "=r"(cr2));
  return cr2;
}
static inline void flush_tlb(uint64_t page)
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

static inline void write_msr(uint32_t msr, uint32_t low, uint32_t high)
{
  asm volatile("wrmsr"
    :
    : "a"(low), "d"(high), "c"(msr));
}

static inline void outb(uint16_t port, uint8_t val)
{
  __asm__ volatile("outb %b0, %1"
    :
    : "a"(val), "Nd"(port)
    : "memory");
}

static inline uint8_t inb(uint16_t port)
{
  uint8_t val;
  __asm__ volatile("inb %1, %b0"
    : "=a"(val)
    : "Nd"(port)
    : "memory");
  return val;
}

#endif
