// clang-format Language: C
#ifndef _STDIO_H
#define _STDIO_H

#include <stdint.h>
#include <sys/cdefs.h>
#define EOF (-1)
int printf(const char* __restrict, ...);
int putchar(int);
int puts(const char*);
int outb(uint16_t port, uint8_t val);
#endif
