#include "aml.h"
#include "parser.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

// returns aml_ptr UNUSED, name_segments(1)
aml_ptr_t parse_name_seg()
{
  uint8_t lead_char       = next_byte();
  uint8_t first_namechar  = next_byte();
  uint8_t second_namechar = next_byte();
  uint8_t third_namechar  = next_byte();
  if (LEAD_CHAR_OOB(lead_char) || NAME_CHAR_OOB(first_namechar) ||
    NAME_CHAR_OOB(second_namechar) || NAME_CHAR_OOB(third_namechar))
    return AML_PREFIX_ERROR;
  aml_name_segment_t* name_segment = calloc(1, sizeof(aml_name_segment_t));

  name_segment->lead_char   = lead_char;
  name_segment->name_char_1 = first_namechar;
  name_segment->name_char_2 = second_namechar;
  name_segment->name_char_3 = third_namechar;
  return (aml_ptr_t){NAME_SEG_PREFIX, name_segment};
}

// returns aml_ptr DUAL_NAME_PREFIX, name_segments(2)
aml_ptr_t parse_dual_name_path()
{
  AML_PRELUDE(DUAL_NAME_PREFIX);
  aml_ptr_t first_seg = parse_name_seg();
  AML_ERR_CHECK(first_seg);
  aml_ptr_t second_seg = parse_name_seg();
  AML_ERR_CHECK(second_seg);
  aml_dual_name_path_t* name_path = calloc(1, sizeof(aml_dual_name_path_t));
  name_path->first                = *(aml_name_segment_t*)first_seg.__ptr;
  name_path->second               = *(aml_name_segment_t*)second_seg.__ptr;
  free(first_seg.__ptr);
  free(second_seg.__ptr);
  return (aml_ptr_t){DUAL_NAME_PREFIX, name_path};
}

// returns aml_ptr MULTI_NAME_PREFIX, name_segment(n) where n is an arbitrary
// number of name segments
aml_ptr_t parse_multi_name_path()
{
  AML_PRELUDE(MULTI_NAME_PREFIX);
  uint8_t num_name_segs            = next_byte();
  aml_multi_name_path_t* name_path = calloc(1, sizeof(aml_multi_name_path_t));
  name_path->segments = calloc(num_name_segs, sizeof(aml_name_segment_t));
  for (int i = 0; i < num_name_segs; i++)
  {
    aml_ptr_t name_segment = parse_name_seg();
    AML_ERR_CHECK(name_segment);
    name_path->segments[i] = *(aml_name_segment_t*)name_segment.__ptr;
    free(name_segment.__ptr);
  }
  name_path->length = num_name_segs;

  return (aml_ptr_t){MULTI_NAME_PREFIX, name_path};
}

aml_ptr_t parse_null_name()
{
  AML_PRELUDE(NULL_NAME);
  return (aml_ptr_t){NULL_NAME, NULL};
}

aml_ptr_t parse_name_path()
{
  return one_of(4, parse_null_name, parse_multi_name_path, parse_dual_name_path,
    parse_name_seg);
}

aml_ptr_t parse_super_name()
{
  return one_of(3, parse_misc_obj, reference_type_opcode, parse_name_string);
}

aml_ptr_t parse_target()
{
  return one_of(2, parse_super_name, parse_null_name);
}

aml_ptr_t parse_name_string()
{
  uint8_t token = peek_byte();
  if (token == ROOT_CHAR)
  {
    next_byte(); // consume ROOT_CHAR byte
    aml_ptr_t name_path = parse_name_path();
    return name_path;
  }
  else
  {
    while (token == PREFIX_CHAR)
    {
      token = next_byte();
    }
    aml_ptr_t name_path = parse_name_path();
    AML_ERR_CHECK(name_path);
    return name_path;
  }
}

void print_name_seg(aml_name_segment_t segment)
{
  printf("%c%c%c%c", segment.lead_char, segment.name_char_1,
    segment.name_char_2, segment.name_char_3);
}

void print_name_string(aml_ptr_t string)
{
  switch (string.prefix_byte)
  {
    case NAME_SEG_PREFIX:
      {
        print_name_seg(*(aml_name_segment_t*)string.__ptr);
        break;
      }
    case DUAL_NAME_PREFIX:
      {
        aml_dual_name_path_t* name_path = string.__ptr;
        print_name_seg(name_path->first);
        print_name_seg(name_path->second);
        break;
      }
    case MULTI_NAME_PREFIX:
      {
        aml_multi_name_path_t* name_path = string.__ptr;
        for (int i = 0; i < name_path->length; i++)
        {
          print_name_seg(name_path->segments[i]);
        }
        break;
      }
    case NULL_NAME:
      {
        printf("NullName");
        break;
      }
    default:
      {
        // Name might be SuperName
        // try evaluating it
        aml_ptr_t evaluated_string = evaluate_term_arg(string);
        print_term_arg(evaluated_string);
        break;
      }
  }
}
