#include "aml.h"
#include "parser.h"
#include "stdlib.h"
#include <stdint.h>
#include <string.h>

aml_ptr_t byte_data()
{
  uint8_t token = next_byte();
  uint8_t* ptr  = calloc(1, sizeof(uint8_t));
  *ptr          = token;
  return (aml_ptr_t){BYTE_PREFIX, ptr};
}

aml_ptr_t byte_const()
{
  AML_PRELUDE(BYTE_PREFIX)
  return byte_data();
}

aml_ptr_t word_data()
{
  aml_ptr_t lower_byte = byte_data();
  uint8_t lower        = *(uint8_t*)lower_byte.__ptr;
  aml_ptr_t upper_byte = byte_data();
  uint8_t upper        = *(uint8_t*)upper_byte.__ptr;
  uint16_t* ptr        = calloc(1, sizeof(uint16_t));
  *ptr                 = ((uint16_t)upper << 8) | lower;

  free(lower_byte.__ptr);
  free(upper_byte.__ptr);

  return (aml_ptr_t){WORD_PREFIX, ptr};
}

aml_ptr_t word_const()
{
  AML_PRELUDE(WORD_PREFIX)
  return word_data();
}
// TODO fix qword and dword
aml_ptr_t dword_data()
{
  aml_ptr_t lower_word = word_data();
  aml_ptr_t upper_word = word_data();
  uint16_t lower       = *(uint16_t*)lower_word.__ptr;
  uint16_t upper       = *(uint16_t*)upper_word.__ptr;
  uint32_t* ptr        = calloc(1, sizeof(uint32_t));
  *ptr                 = ((uint32_t)upper << 16) | lower;

  free(lower_word.__ptr);
  free(upper_word.__ptr);

  return (aml_ptr_t){DWORD_PREFIX, ptr};
}

aml_ptr_t dword_const()
{
  AML_PRELUDE(DWORD_PREFIX)
  return dword_data();
}

aml_ptr_t qword_data()
{
  aml_ptr_t lower_word = dword_data();
  aml_ptr_t upper_word = dword_data();
  uint32_t lower       = *(uint32_t*)lower_word.__ptr;
  uint32_t upper       = *(uint32_t*)upper_word.__ptr;
  uint64_t* ptr        = calloc(1, sizeof(uint64_t));
  *ptr                 = ((uint64_t)upper << 32) | lower;

  free(lower_word.__ptr);
  free(upper_word.__ptr);

  return (aml_ptr_t){QWORD_PREFIX, ptr};
}

aml_ptr_t qword_const()
{
  AML_PRELUDE(QWORD_PREFIX)
  return qword_data();
}

aml_ptr_t parse_string()
{
  AML_PRELUDE(STRING_PREFIX)
  char* string  = calloc(5, sizeof(char));
  int size      = 5;
  int i         = 0;
  uint8_t token = next_byte();
  while (token != ZERO_OP)
  {
    if (token > 0x7F)
    {
      free(string);
      return AML_PARSE_ERROR;
    }
    string[i++] = token;
    if (i >= size)
    {
      int new_size     = size * 2;
      char* new_string = calloc(new_size, sizeof(char));
      new_string       = memcpy(new_string, string, size);
      free(string);
      string = new_string;
    }
    token = next_byte();
  }
  if (i >= size)
  {
    char* new_string = calloc(size + 1, sizeof(char));
    new_string       = memcpy(new_string, string, size);
    free(string);
    string = new_string;
  }
  string[i] = 0;

  return (aml_ptr_t){STRING_PREFIX, string};
}

aml_ptr_t const_obj()
{
  uint8_t token = next_byte();
  if (token == ZERO_OP || token == ONE_OP || token == ONES_OP)
  {
    return (aml_ptr_t){token, NULL};
  }
  return AML_PREFIX_ERROR;
}

aml_ptr_t revision_op()
{
  AML_EXT_PRELUDE(REVISION_OP)
  return (aml_ptr_t){REVISION_OP, NULL};
}

aml_ptr_t computational_data()
{
  return one_of(8, byte_const, word_const, dword_const, qword_const,
    parse_string, const_obj, revision_op, def_buffer);
}
