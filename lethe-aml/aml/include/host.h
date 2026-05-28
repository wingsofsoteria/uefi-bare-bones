#ifndef __AML_INTERNAL_HOST_H__
#define __AML_INTERNAL_HOST_H__
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#ifndef __is_libk

static void host_exit() { exit(1); }

inline static void outb(uint16_t port, uint64_t val) {}

inline static uint8_t inb(uint16_t port) { return 0; }

  #define outw outb
  #define inw  inb
  #define outd outb
  #define ind  inb
  #define outq outb
  #define inq  inb
  #define alog(...)
#else
extern void walk_stack();

static void host_exit()
{
  walk_stack();
  abort();
}

extern void* malloc(size_t);
extern void* calloc(size_t, int);
extern void  free(void*);
  #include <log.h>
  #include <utils.h>
  #define alog(...) __kernel_log(__source, __VA_ARGS__)
#endif

#define unimplemented(expr)                           \
  if (expr) { printf("Unimplemented! %s\n", #expr); } \
  assert(!(expr))
#define debug_code(x, n)                                     \
  for (int i = 0; i < n; i++) { printf("%x ", x->code[i]); } \
  printf("\n");

#define AML_EXIT()                \
  printf("Exiting AML parser\n"); \
  host_exit();
#endif
