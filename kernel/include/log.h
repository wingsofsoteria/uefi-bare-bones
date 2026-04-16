#pragma once
#include "stdarg.h"
enum
{
  KERNEL_LOG_DEBUG = 1,
  KERNEL_LOG_ERROR = 5,
};
int kernel_log_debug(char*, ...);
int kernel_log_error(char*, ...);
void kernel_init_logging(int);
int kernel_log(int, char*, ...);
int kernel_vlog(int, char*, va_list);
