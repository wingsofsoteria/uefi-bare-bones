// clang-format Language: C
#ifndef _STDLIB_H
#define _STDLIB_H

#include <stddef.h>
#include <sys/cdefs.h>

#define halt_cpu         \
  for (;;)               \
  {                      \
    asm volatile("hlt"); \
  }

__attribute__((__noreturn__)) void __abort(void);
__attribute__((__noreturn__)) void __abort_msg(const char* fmt, ...);

#endif
