#ifndef __AML_INTERNAL_HOST_H__
#define __AML_INTERNAL_HOST_H__
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

static void host_exit()
{
  abort();
}

#ifdef AML_DEBUG // AML_DEBUG can be defined for the host OS and the kernel so
                 // we can't make any assumptions beyond the existence of a
                 // printf function
  #define AML_LOG(...) printf(__VA_ARGS__)
#else
  #define AML_LOG(...)
#endif
#define AML_EXIT()                 \
  AML_LOG("Exiting AML parser\n"); \
  host_exit();
#endif
