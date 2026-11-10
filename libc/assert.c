#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

void __assert(bool expr, struct source_location location, const char* expr_str)
{
  if (!expr)
    {
      printf(
        "Assertion %s failed at %s:%d (%s)\n",
        expr_str,
        location.file,
        location.line,
        location.function
      );
      abort();
    }
}
