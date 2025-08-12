// clang-format Language: C
#ifndef __KERNEL_QEMU_H__
#define __KERNEL_QEMU_H__
#include <stdio.h>
#define debug(fmt, ...)          \
  qemu_printf(fmt, __VA_ARGS__); \
  printf(fmt, __VA_ARGS__)

#define debug_empty(fmt) \
  qemu_printf(fmt);      \
  printf(fmt)
int qemu_printf(const char* restrict format, ...);

#endif
