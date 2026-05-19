// clang-format Language: C
#ifndef _STDIO_H
#define _STDIO_H

#include <stdarg.h>
#define EOF (-1)
int                                       vprintf(const char*, va_list);
__attribute__((format(printf, 1, 2))) int printf(const char* __restrict, ...);
int                                       putchar(int);
int                                       puts(const char*);
#endif
