#pragma once
#include "stdarg.h"
#include "stdio.h"
enum {
  KERNEL_LOG_DEBUG = 1,
  KERNEL_LOG_ERROR = 5,
};

#define __kernel_log(level, ...)                                               \
  kernel_log(level, "[%s:%d] ", __func__, __LINE__);                           \
  kernel_log(level, __VA_ARGS__);                                              \
  putchar('\n');

#define kernel_log_error(...) __kernel_log(KERNEL_LOG_ERROR, __VA_ARGS__);
#define kernel_log_debug(...) __kernel_log(KERNEL_LOG_DEBUG, __VA_ARGS__);
// int kernel_log_debug(char*, ...);
// int kernel_log_error(char*, ...);
void                                      kernel_init_logging(int);
__attribute__((format(printf, 2, 3))) int kernel_log(int, char*, ...);
int                                       kernel_vlog(int, char*, va_list);
