// clang-format Language: C
#ifndef _STDLIB_H
#define _STDLIB_H

#include <sys/cdefs.h>

__attribute__((__noreturn__)) static inline void halt()
{
  asm("cli");
  for (;;) { asm volatile("hlt"); }
  __builtin_unreachable();
}

__attribute__((__noreturn__)) void abort(void);
#endif
