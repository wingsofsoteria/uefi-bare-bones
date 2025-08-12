#include <stdio.h>
#include <stdlib.h>
#include "../kernel/memory/liballoc.h"

__attribute__((__noreturn__)) void abort(void)
{
  printf("kernel panicked\n");
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
