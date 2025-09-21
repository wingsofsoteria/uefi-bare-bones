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
__attribute__((__noreturn__)) void abort(void);
void* malloc(size_t size);

void free(void* ptr);

void* calloc(size_t nobj, size_t size);

void* realloc(void* p, size_t size);

#endif
