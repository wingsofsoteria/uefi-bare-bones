#include "dsdt.h"
void aml_parser_init(void*);
void aml_parser_run();
int main()
{
  aml_parser_init((void*)dsdt_dat);
  aml_parser_run();
  return 0;
}
