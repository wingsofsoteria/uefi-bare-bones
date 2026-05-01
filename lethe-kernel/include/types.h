#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
/*#ifdef KERNEL_USE_LIMINE
  #define KERNEL_START 0xffffffff80000000
#else
  #define KERNEL_START 0xFFFF800000000000
#endif
*/
#define ALIGN_UP(x, a)   ((x + a - 1) & ~(a - 1))
#define ALIGN_DOWN(x, a) (x & ~a)
#define PAGE_ALIGN(x)    (x & ~4096)
extern uint64_t hhdm_mapping;
