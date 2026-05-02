#include <stdio.h>
#include <stdlib.h>

#if defined(__is_libk)
  #include "../lethe-kernel/include/utils.h"

__attribute__((__noreturn__)) void abort() { halt(); }
#endif
/*
extern void walk_stack(void);

__attribute__((__noreturn__)) void __abort_msg(const char *fmt, ...)
{
  asm("cli");
  va_list args;
  va_start(args, fmt);
  vprintf(fmt, args);
  va_end(args);
  walk_stack();
  halt_cpu;
  __builtin_unreachable();
}

__attribute__((__noreturn__)) void __abort()
{
  asm("cli");
  walk_stack();
  halt_cpu;
  __builtin_unreachable();
}
*/
