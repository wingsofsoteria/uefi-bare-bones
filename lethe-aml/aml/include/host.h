#ifndef __AML_INTERNAL_HOST_H__
#define __AML_INTERNAL_HOST_H__

#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct source_location source_location;

extern void* scan_tables(const char*, size_t);
int          __aml_log(struct source_location, char*, ...);
#define aml_log(...) __aml_log(__source, __VA_ARGS__)

#ifndef __is_libk

struct source_location
{
  const char* file;
  const char* function;
  uint32_t    line;
};

#define __source \
  (struct source_location) { __FILE__, __func__, __LINE__ }

static void host_exit() { exit(1); }

inline static void outb(uint16_t port, uint64_t val) {}

inline static uint8_t inb(uint16_t port) { return 0; }

#define outw outb
#define inw  inb
#define outd outb
#define inl  inb
#define outq outb
#define inq  inb

static void walk_stack() {}

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
#include <utils.h>

#endif

#define unimplemented(expr)                  \
  if (expr)                                  \
    {                                        \
      aml_log("Unimplemented! %s\n", #expr); \
      walk_stack();                          \
    }                                        \
  assert(!(expr))
#define debug_code(ns, count)                                     \
  for (int i = 0; i < count; i++) { printf("%x ", ns->code[i]); } \
  aml_log("\n");
#define AML_EXIT()                 \
  aml_log("Exiting AML parser\n"); \
  host_exit();

#endif
