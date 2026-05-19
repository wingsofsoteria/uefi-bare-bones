#ifndef __AML_INTERNAL_HOST_H__
#define __AML_INTERNAL_HOST_H__
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void  abort();
void* calloc(size_t, size_t);
void  free(void*);
void* malloc(size_t);
void* realloc(void*, size_t);

#ifndef __is_libk
static void host_exit() { exit(1); }
#else
extern void walk_stack();

static void host_exit()
{
  walk_stack();
  abort();
}
#endif
#define unimplemented(expr)                           \
  if (expr) { printf("Unimplemented! %s\n", #expr); } \
  assert(!(expr))

#ifdef __is_libk
  #include <log.h>
  #include <utils.h>
  #define alog(...) __kernel_log(__source, __VA_ARGS__)
#else
  #define alog(...) printf(__VA_ARGS__)

inline static void outb(uint16_t port, uint64_t val) {}

inline static uint8_t inb(uint16_t port) { return 0; }

  #define outw outb
  #define inw  inb
  #define outd outb
  #define ind  inb
  #define outq outb
  #define inq  inb
#endif

#define AML_EXIT()                \
  printf("Exiting AML parser\n"); \
  host_exit();
#endif
