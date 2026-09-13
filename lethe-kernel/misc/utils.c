#include "utils.h"

#include "assert.h"

#include <stdio.h>

__attribute__((__noreturn__)) void __panic(
  char*                  msg,
  struct source_location location
)
{
  printf(
    "Panicked at %s:%d (%s): %s",
    location.file,
    location.line,
    location.function,
    msg
  );
  walk_stack();
  halt();
}
