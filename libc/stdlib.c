#include <stdio.h>
#include <stdlib.h>

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
  return NULL;
  // return kernel_malloc(size);
}

void free(void* ptr)
{
}

void* calloc(size_t nobj, size_t size)
{
  return NULL;
}

void* realloc(void* p, size_t size)
{
  return NULL;
}
