#ifndef __KERNEL_LOG_H__
#define __KERNEL_LOG_H__

enum
{
  KERNEL_LOG_SILENT = 0,
  KERNEL_LOG_DEBUG  = 1,
  KERNEL_LOG_ERROR  = 2,
};

int kernel_log_debug(char*, ...);
int kernel_log_error(char*, ...);
void kernel_init_logging(int);
#endif
