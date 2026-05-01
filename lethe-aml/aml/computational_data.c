#include "aml.h"
#include "parser.h"
#include "stdlib.h"
#include <stdint.h>
#include <string.h>

static aml_ptr_t byte_data()
{
  uint8_t token = next_byte();
  uint8_t* ptr  = calloc(1, sizeof(uint8_t));
  *ptr          = token;
  return (aml_ptr_t){BYTE_PREFIX, ptr};
}

static aml_ptr_t word_data()
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

// TODO fix qword and dword
static aml_ptr_t dword_data()
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

static aml_ptr_t qword_data()
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

static aml_ptr_t parse_string()
{
  int size      = 5;
  char* string  = calloc(size, sizeof(char));
  int i         = 0;
  uint8_t token = next_byte();
  while (token != ZERO_OP)
  {
    if (token > STRING_OOB)
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

aml_ptr_t computational_data()
{
  uint8_t token = next_byte();
  switch (token)
  {
    case EXT_OP_PREFIX:
      {
        token = next_byte();
        if (token != EXT_REVISION_OP)
        {
          return AML_PREFIX_ERROR;
        }
        return (aml_ptr_t){EXT_REVISION_OP, NULL};
      }
    case BYTE_PREFIX:
      return byte_data();
    case WORD_PREFIX:
      return word_data();
    case DWORD_PREFIX:
      return dword_data();
    case QWORD_PREFIX:
      return qword_data();
    case STRING_PREFIX:
      return parse_string();
    case ZERO_OP:
    case ONE_OP:
    case ONES_OP:
      return (aml_ptr_t){token, NULL};
    case BUFFER_OP:
      return def_buffer();
    default:
      return AML_PREFIX_ERROR;
  }
}
