#include "aml.h"
#include "host.h"
#include "parser.h"
#include <stdint.h>

static int compare_method_names(const char expected_name[4], aml_ptr_t aml_name)
{
  if (aml_name.__ptr == NULL)
  {
    return 1;
  }
  aml_name_string_t* test_name = (aml_name_string_t*)aml_name.__ptr;
  if (aml_name.prefix_byte == ERR_PREFIX || aml_name.prefix_byte == ERR_PARSE)
  {
    return 1;
  }
  // we might need to support dual / multi segments later but for now this is
  // fine
  if (test_name->name_prefix != NAME_SEG_PREFIX)
  {
    return 1;
  }
  aml_name_segment_t name = test_name->name_seg;
  if (name.lead_char == expected_name[0] &&
    name.name_char_1 == expected_name[1] &&
    name.name_char_2 == expected_name[2] &&
    name.name_char_3 == expected_name[3])
  {
    return 0;
  }
  return 1;
}

static int lookup_method(const char method_name[4])
{
  while (1)
  {
    uint8_t byte = next_byte();
    while (byte != METHOD_OP)
    {
      if (get_pointer() >= table_length())
      {
        return_TO_ANCHOR(1);
      }
      byte = next_byte();
    }
    if (get_pointer() >= table_length())
    {
      return_TO_ANCHOR(1);
    }
    AML_SET_ANCHOR;
    parse_pkg_length();
    aml_ptr_t aml_name = parse_name_string();
    if (aml_name.prefix_byte == ERR_PARSE || aml_name.prefix_byte == ERR_PREFIX)
    {
      continue;
    }
    if (compare_method_names(method_name, aml_name) == 0)
    {
      break;
    }
  }
  return_TO_ANCHOR(0);
}

static void exec_method(const char method_name[4])
{
  AML_SET_ANCHOR;
  int status = lookup_method(method_name); // this should set the pointer to the
                                           // beginning of the method definition
  if (status == 1)
  {
    AML_LOG("Method %.4s not found\n", method_name);
    return_TO_ANCHOR();
  }
  int current_pointer   = get_pointer();
  uint32_t length       = parse_pkg_length();
  aml_ptr_t aml_name    = parse_name_string();
  uint8_t method_flags  = next_byte();
  int new_pointer       = get_pointer();
  length               -= (new_pointer - current_pointer);
  print_next_definition_block();
  AML_EXIT();
}

void __pts(int sleep_state)
{
  exec_method("_PTS");
}
