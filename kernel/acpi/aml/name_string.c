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
  aml_name_segment_t name_segment;
  aml_name_string_t* name_string = calloc(1, sizeof(aml_name_string_t));

  name_segment.lead_char   = lead_char;
  name_segment.name_char_1 = first_namechar;
  name_segment.name_char_2 = second_namechar;
  name_segment.name_char_3 = third_namechar;

  name_string->name_prefix = NAME_SEG_PREFIX;
  name_string->name_seg    = name_segment;
  return (aml_ptr_t){NAME_SEG_PREFIX, name_string};
}

// returns aml_ptr DUAL_NAME_PREFIX, name_segments(2)
aml_ptr_t parse_dual_name_path()
{
  AML_PRELUDE(DUAL_NAME_PREFIX);
  aml_ptr_t first_seg = parse_name_seg();
  AML_ERR_CHECK(first_seg);
  aml_ptr_t second_seg = parse_name_seg();
  AML_ERR_CHECK(second_seg);
  aml_dual_name_path_t dual_seg;
  aml_name_string_t* name_string = calloc(1, sizeof(aml_name_string_t));
  dual_seg.first           = ((aml_name_string_t*)first_seg.__ptr)->name_seg;
  dual_seg.second          = ((aml_name_string_t*)second_seg.__ptr)->name_seg;
  name_string->name_prefix = DUAL_NAME_PREFIX;
  name_string->dual_seg    = dual_seg;
  free(first_seg.__ptr);
  free(second_seg.__ptr);
  return (aml_ptr_t){DUAL_NAME_PREFIX, name_string};
}

// returns aml_ptr MULTI_NAME_PREFIX, name_segment(n) where n is an arbitrary
// number of name segments
aml_ptr_t parse_multi_name_path()
{
  AML_PRELUDE(MULTI_NAME_PREFIX);
  uint8_t num_name_segs = next_byte();

  aml_multi_name_path_t multi_seg;
  multi_seg.segments = calloc(num_name_segs, sizeof(aml_name_segment_t));
  for (int i = 0; i < num_name_segs; i++)
  {
    aml_ptr_t name_segment = parse_name_seg();
    AML_ERR_CHECK(name_segment);
    multi_seg.segments[i] = ((aml_name_string_t*)name_segment.__ptr)->name_seg;
    free(name_segment.__ptr);
  }
  multi_seg.length               = num_name_segs;
  aml_name_string_t* name_string = calloc(1, sizeof(aml_name_string_t));
  name_string->name_prefix       = MULTI_NAME_PREFIX;
  name_string->multi_seg         = multi_seg;
  return (aml_ptr_t){MULTI_NAME_PREFIX, name_string};
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
    AML_ERR_CHECK(name_path);
    return (aml_ptr_t){ROOT_CHAR, name_path.__ptr};
  }
  else
  {
    while (token == PREFIX_CHAR)
    {
      token = next_byte();
    }
    aml_ptr_t name_path = parse_name_path();
    AML_ERR_CHECK(name_path);
    return (aml_ptr_t){PREFIX_CHAR, name_path.__ptr};
  }
}

char* name_string_to_cstring(aml_ptr_t name_string)
{
  if (name_string.prefix_byte != PREFIX_CHAR &&
    name_string.prefix_byte != ROOT_CHAR)
    return NULL;
  if (name_string.__ptr == NULL) return "\\";
  aml_name_string_t name_path = *((aml_name_string_t*)name_string.__ptr);
  switch (name_path.name_prefix)
  {
    case MULTI_NAME_PREFIX:
      {
        aml_multi_name_path_t multi = name_path.multi_seg;
        char* cstring = calloc((multi.length * 4) + 1, sizeof(char));
        for (int i = 0; i < multi.length; i++)
        {
          cstring[(i * 4) + 0] = multi.segments[i].lead_char;
          cstring[(i * 4) + 1] = multi.segments[i].name_char_1;
          cstring[(i * 4) + 2] = multi.segments[i].name_char_2;
          cstring[(i * 4) + 3] = multi.segments[i].name_char_3;
        }
        cstring[multi.length * 4] = 0;
        return cstring;
      }
    case DUAL_NAME_PREFIX:
      {
        aml_dual_name_path_t dual = name_path.dual_seg;

        char* cstring = calloc(9, sizeof(char));
        cstring[0]    = dual.first.lead_char;
        cstring[1]    = dual.first.name_char_1;
        cstring[2]    = dual.first.name_char_2;
        cstring[3]    = dual.first.name_char_3;

        cstring[4] = dual.second.lead_char;
        cstring[5] = dual.second.name_char_1;
        cstring[6] = dual.second.name_char_2;
        cstring[7] = dual.second.name_char_3;
        cstring[8] = 0;
        return cstring;
      }
    case NAME_SEG_PREFIX:
      {
        aml_name_segment_t segment = name_path.name_seg;

        char* cstring = calloc(5, sizeof(char));
        cstring[0]    = segment.lead_char;
        cstring[1]    = segment.name_char_1;
        cstring[2]    = segment.name_char_2;
        cstring[3]    = segment.name_char_3;
        cstring[4]    = 0;
        return cstring;
      }
    default:
      {
        return NULL;
      }
  }
}

void print_name_seg(aml_name_segment_t segment)
{
  printf("%c%c%c%c", segment.lead_char, segment.name_char_1,
    segment.name_char_2, segment.name_char_3);
}

void print_name_string(aml_ptr_t string)
{
  printf("%c",
    string.prefix_byte == ROOT_CHAR || string.prefix_byte == PREFIX_CHAR
      ? string.prefix_byte
      : ' ');
  aml_name_string_t name_string = *((aml_name_string_t*)string.__ptr);
  switch (name_string.name_prefix)
  {
    case NAME_SEG_PREFIX:
      {
        print_name_seg(name_string.name_seg);
        break;
      }
    case DUAL_NAME_PREFIX:
      {
        print_name_seg(name_string.dual_seg.first);
        printf(".");
        print_name_seg(name_string.dual_seg.second);
        break;
      }
    case MULTI_NAME_PREFIX:
      {
        for (int i = 0; i < name_string.multi_seg.length; i++)
        {
          print_name_seg(name_string.multi_seg.segments[i]);
          printf(".");
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
