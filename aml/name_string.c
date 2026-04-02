#include "aml.h"
#include "parser.h"
#include "stdlib.h"

char* last_segment(char* segment)
{
  int length  = __builtin_strlen(segment);
  length     -= 4;
  return (char*)(segment + length);
}

static char* parse_name_seg()
{
  if (LEAD_CHAR_OOB(peek_byte(0)) || NAME_CHAR_OOB(peek_byte(1)) ||
    NAME_CHAR_OOB(peek_byte(2)) || NAME_CHAR_OOB(peek_byte(3)))
  {
    return NULL;
  }

  char* segment = malloc(5 * sizeof(char));
  for (int i = 0; i < 4; i++)
  {
    segment[i] = next_byte();
  }
  segment[4] = 0;
  return segment;
}

static char* parse_dual_name_path()
{
  void* table    = table_ptr();
  char* dual_seg = malloc(9 * sizeof(char));
  __builtin_memcpy(dual_seg, table, 8);
  dual_seg[8] = 0;
  return dual_seg;
}

static char* parse_multi_name_path()
{
  uint8_t segment_count = next_byte();
  int char_count        = segment_count * 4;
  char* multi_seg       = malloc((char_count + 1) * sizeof(char));
  __builtin_memcpy(multi_seg, table_ptr(), char_count);
  multi_seg[char_count] = 0;
  return multi_seg;
}

static aml_ptr_t parse_null_name()
{
  AML_PRELUDE(NULL_NAME);
  return (aml_ptr_t){NULL_NAME, NULL};
}

static aml_ptr_t parse_name_path()
{
  uint8_t token = next_byte();
  switch (token)
  {
    case MULTI_NAME_PREFIX:
      return (aml_ptr_t){MULTI_NAME_PREFIX, parse_multi_name_path()};
    case DUAL_NAME_PREFIX:
      return (aml_ptr_t){DUAL_NAME_PREFIX, parse_dual_name_path()};
    case NULL_NAME:
      {
        return (aml_ptr_t){NULL_NAME, NULL};
      }
    default:
      {
        move_pointer(-1);
        return (aml_ptr_t){NAME_SEG_PREFIX, parse_name_seg()};
      }
  }
}

aml_ptr_t parse_super_name()
{
  aml_ptr_t status;
  TRY_PARSE(parse_misc_obj);
  TRY_PARSE(reference_type_opcode);
  TRY_PARSE(parse_name_string);
  return AML_PREFIX_ERROR;
}

aml_ptr_t parse_target()
{
  uint8_t token = next_byte();
  if (token == NULL_NAME)
  {
    return (aml_ptr_t){NULL_NAME, NULL};
  }

  move_pointer(-1);
  return parse_super_name();
}

aml_ptr_t parse_name_string()
{
  uint8_t token = peek_byte(0);
  if (token == ROOT_CHAR)
  {
    next_byte(); // consume ROOT_CHAR byte
    aml_ptr_t name_path = parse_name_path();
    AML_ERR_CHECK(name_path);

    return (aml_ptr_t){ROOT_CHAR, name_path.__ptr};
  }

  while (token == PREFIX_CHAR)
  {
    token = next_byte();
  }
  aml_ptr_t name_path = parse_name_path();
  AML_ERR_CHECK(name_path);
  return (aml_ptr_t){PREFIX_CHAR, name_path.__ptr};
}
