// clang-format Language: C
#ifndef _STDIO_H
#define _STDIO_H

#include <stdint.h>
#include <sys/cdefs.h>
#define EOF (-1)
int printf(const char* __restrict, ...);
int putchar(int);
int puts(const char*);

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
