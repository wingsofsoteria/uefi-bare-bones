// clang-format Language: C
#ifndef _STDIO_H
#define _STDIO_H

#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/cdefs.h>

#define EOF (-1)
#ifdef KERNEL_DEBUG
  #define LOG_DEBUG(fmt, ...)   \
    printf("[%s] ", __func__);  \
    printf(fmt, ##__VA_ARGS__);
#else
  #define LOG_DEBUG(fmt, ...)
#endif
#define abort_msg(format, ...)        \
  printf("ABORT [%s]: ", __func__);   \
  __abort_msg(format, ##__VA_ARGS__);

#define abort()                   \
  printf("ABORT [%s]", __func__); \
  __abort();

int vprintf(const char* restrict format, va_list parameters);
int printf(const char* __restrict, ...);
int putchar(int);
int puts(const char*);

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
