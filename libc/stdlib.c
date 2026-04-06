#include <stdio.h>
#include <stdlib.h>
#include <memory/liballoc.h>

__attribute__((__noreturn__)) void __abort_msg(const char* fmt, ...)
{
  asm("cli");
  va_list args;
  va_start(args, fmt);
  vprintf(fmt, args);
  va_end(args);
  halt_cpu;
  __builtin_unreachable();
}

__attribute__((__noreturn__)) void __abort()
{
  asm("cli");
  halt_cpu;
  __builtin_unreachable();
}

void* malloc(size_t size)
{
  return kernel_malloc(size);
}

void free(void* ptr)
{
  kernel_free(ptr);
}

void* calloc(size_t nobj, size_t size)
{
  return kernel_calloc(nobj, size);
}

void* realloc(void* p, size_t size)
{
  return kernel_realloc(p, size);
}
