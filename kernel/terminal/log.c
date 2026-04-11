#include "log.h"
#include <stdio.h>

const static char* kernel_log_str[3] = {"", "debug", "error"};
static int kernel_max_log_level      = KERNEL_LOG_SILENT;

void kernel_init_logging(int level)
{
  kernel_max_log_level = level;
}

static int kernel_vlog(int kernel_log_level, char* fmt, va_list args)
{
  if (kernel_log_level < kernel_max_log_level)
  {
    return 0;
  }
  printf("(%s) ", kernel_log_str[kernel_log_level]);
  int written = vprintf(fmt, args);
  putchar('\n');
  return written;
}

int kernel_log_debug(char* fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  int written = kernel_vlog(KERNEL_LOG_DEBUG, fmt, args);
  va_end(args);
  return written;
}

int kernel_log_error(char* fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  int written = kernel_vlog(KERNEL_LOG_ERROR, fmt, args);
  va_end(args);
  return written;
}
