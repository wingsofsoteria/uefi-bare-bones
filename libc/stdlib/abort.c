#include <stdio.h>
#include <stdlib.h>

__attribute__((__noreturn__)) void abort(void)
{
  printf("kernel panicked\n");
  asm volatile("hlt");
  while (1)
  {
  }
  __builtin_unreachable();
}
