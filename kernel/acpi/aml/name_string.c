#include "aml.h"
#include "parser.h"
#include <stdio.h>

// returns aml_ptr UNUSED, name_segments(1)
aml_ptr_t parse_name_seg()
{
  uint8_t lead_char       = next_byte();
  uint8_t first_namechar  = next_byte();
  uint8_t second_namechar = next_byte();
  uint8_t third_namechar  = next_byte();
  if (LEAD_CHAR_OOB(lead_char) || NAME_CHAR_OOB(first_namechar) ||
    NAME_CHAR_OOB(second_namechar) || NAME_CHAR_OOB(third_namechar))
    return AML_ERROR;
  printf("NameSeg %c%c%c%c ", lead_char, first_namechar, second_namechar,
    third_namechar);
  aml_name_segment_t* name_segment_ptr = calloc(1, sizeof(aml_name_segment_t));
  aml_name_segment_t name_segment      = (aml_name_segment_t){
    lead_char, first_namechar, second_namechar, third_namechar};
  *name_segment_ptr = name_segment;
  return (aml_ptr_t){
    NULL_NAME, name_segment_ptr}; // we don't care about the prefix unless its
                                  // AML_PREFIX_ERROR
}

// returns aml_ptr DUAL_NAME_PREFIX, name_segments(2)
aml_ptr_t parse_dual_name_path()
{
  uint8_t token = next_byte();
  if (token != DUAL_NAME_PREFIX) return AML_ERROR;
  printf("DualNamePath ");
  aml_ptr_t first_seg = parse_name_seg();
  if (first_seg.prefix_byte == AML_PREFIX_ERROR) return AML_ERROR;
  aml_ptr_t second_seg = parse_name_seg();
  if (second_seg.prefix_byte == AML_PREFIX_ERROR) return AML_ERROR;

  aml_name_segment_t* name_path = calloc(2, sizeof(aml_name_segment_t));
  name_path[0] =
    *(aml_name_segment_t*)
       first_seg.__ptr; // TODO same possible errors as multi_name_path
  name_path[1] = *(aml_name_segment_t*)second_seg.__ptr;
  free(first_seg.__ptr);
  free(second_seg.__ptr);
  return (aml_ptr_t){DUAL_NAME_PREFIX, name_path};
}

// returns aml_ptr MULTI_NAME_PREFIX, name_segment(n) where n is an arbitrary
// number of name segments
aml_ptr_t parse_multi_name_path()
{
  uint8_t token = next_byte();
  if (token != MULTI_NAME_PREFIX) return AML_ERROR;
  printf("MultiNamePath ");
  uint8_t num_name_segs = next_byte();
  aml_name_segment_t* name_path =
    calloc(num_name_segs, sizeof(aml_name_segment_t));
  for (int i = 0; i < num_name_segs; i++)
  {
    aml_ptr_t name_segment = parse_name_seg();
    if (name_segment.prefix_byte == AML_PREFIX_ERROR) return AML_ERROR;
    aml_name_segment_t* name_seg_ptr = name_segment.__ptr;
    name_path[i] = *name_seg_ptr; // TODO possible error, might need to memcpy
    free(name_seg_ptr);           // TODO unlikely error based on previous TODO
  }

  return (aml_ptr_t){MULTI_NAME_PREFIX, name_path};
}

aml_ptr_t parse_null_name()
{
  uint8_t token = next_byte();
  if (token != NULL_NAME) return AML_ERROR;
  printf("NullName ");
  return (aml_ptr_t){NULL_NAME, NULL};
}

aml_ptr_t parse_name_path()
{
  return one_of(4, parse_null_name, parse_multi_name_path, parse_dual_name_path,
    parse_name_seg);
}

aml_ptr_t parse_super_name()
{
  return one_of(5, parse_local_obj, parse_arg_obj, parse_debug_obj,
    reference_type_opcode, parse_name_string);
}

aml_ptr_t parse_target()
{
  return one_of(2, parse_super_name, parse_null_name);
}

aml_ptr_t parse_name_string()
{
  uint8_t token = next_byte();
  if (token == ROOT_CHAR)
  {
    aml_ptr_t name_path = parse_name_path();
    return (aml_ptr_t){ROOT_CHAR, name_path.__ptr};
  }
  else
  {
    while (token == PREFIX_CHAR)
    {
      token = next_byte();
    }
    decrement_pointer();
    aml_ptr_t name_path = parse_name_path();
    if (name_path.prefix_byte == AML_PREFIX_ERROR)
    {
      printf("NameString ERROR ");
      abort(); // name_string parser has a bug
    }
    return (aml_ptr_t){PREFIX_CHAR, name_path.__ptr};
  }
}
