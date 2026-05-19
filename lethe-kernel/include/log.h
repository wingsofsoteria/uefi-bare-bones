#pragma once
#include "assert.h"

#define klog(...) __kernel_log(__source, __VA_ARGS__)

__attribute__((format(printf, 2, 3))) int __kernel_log(
  struct source_location,
  char*,
  ...
);
