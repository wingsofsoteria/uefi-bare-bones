#pragma once
#include "stdarg.h"
enum
{
  KERNEL_LOG_DEBUG = 5,
  KERNEL_LOG_ERROR = 1,
};

int kernel_log_debug(char*, ...);
int kernel_log_error(char*, ...);
void kernel_init_logging(int);
int kernel_vlog(int, char*, va_list);
