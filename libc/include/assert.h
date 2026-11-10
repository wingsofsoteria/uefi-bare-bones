#pragma once

#include <stdbool.h>
#include <stdint.h>

struct source_location
{
  const char* file;
  const char* function;
  uint32_t    line;
};

#define __source \
  (struct source_location) { __FILE__, __func__, __LINE__ }

#define assert(expr) __assert(expr, __source, #expr)

void __assert(bool expr, struct source_location location, const char* expr_str);
