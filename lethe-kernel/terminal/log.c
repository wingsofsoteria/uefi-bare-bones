#include "log.h"

#include "assert.h"

#include <stdarg.h>
#include <stdio.h>

static const char* kernel_log_str[6] = { "",
  "debug",
  "warn",
  "trace",
  "info",
  "error" };

// const static char* kernel_log_str[6] = {
//"", "error", "warn", "info", "trace", "debug"};

int __kernel_log(struct source_location location, char* fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  printf("[%s:%d] ", location.function, location.line);
  int written = vprintf(fmt, args);
  va_end(args);
  return written;
}
