// clang-format Language: C
#ifndef __KERNEL_GRAPHICS_DEBUG_H__
#define __KERNEL_GRAPHICS_DEBUG_H__
#include <stdio.h>
#define debug(fmt, ...)         \
  printf(fmt, __VA_ARGS__);     \
  qemu_printf(fmt, __VA_ARGS__)

#define debug_empty(fmt) \
  printf(fmt);           \
  qemu_printf(fmt)

int qemu_printf(const char* restrict format, ...);

#endif
