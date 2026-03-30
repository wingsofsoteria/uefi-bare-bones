#include "dsdt.h"
void aml_parser_init(void*);
void aml_parser_run();
void __pts(int);
int main()
{
  aml_parser_init((void*)dsdt_dat);
  __pts(5);
  // aml_parser_run();
  return 0;
}
