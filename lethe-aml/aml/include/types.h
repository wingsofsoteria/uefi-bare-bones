#pragma once

#include "hashmap.h"

#include <stdint.h>

typedef struct aml_namespace
{
  struct aml_namespace* parent;
  hash_key              name;
  uint8_t*              code;
  hash_map_t*           children;
  hash_map_t*           namespaces;
} aml_namespace_t;

#define MAX_CHARS 1022

typedef struct aml_name
{
  size_t count;
  char*  inner;
} aml_name_t;

typedef struct
{
  hash_key label;
  uint8_t  data_type;

  union
  {
    uint8_t               byte;
    uint16_t              short_int;
    uint32_t              integer;
    uint64_t              long_int;
    struct aml_package_t* package;
    char*                 string;
    struct aml_buffer_t*  buffer;
  };
} aml_variable_t;

typedef struct
{
  hash_key name;
  uint8_t  sync_flags;
} aml_mutex_t;

typedef struct aml_buffer_t
{
  size_t   size;
  uint8_t* buffer;
} aml_buffer_t;

typedef struct aml_package_t
{
  size_t           num_elements;
  aml_variable_t** elements;
} aml_package_t;

typedef struct
{
  hash_key         name;
  uint8_t          flags;
  size_t           len;
  uint8_t*         code;
  aml_namespace_t* scope;
} aml_method_t;

enum VariableType
{
  VariableUnimplemented = 0,
  VariableInt           = 1,
  VariableStr           = 2,
};

enum AccessType
{
  AnyAcc    = 0,
  ByteAcc   = 1,
  WordAcc   = 2,
  DWordAcc  = 3,
  QWordAcc  = 4,
  BufferAcc = 5
};

enum UpdateRule
{
  Preserve     = 0,
  WriteAsOnes  = 1,
  WriteAsZeros = 2
};

enum FieldUnitType
{
  Reserved       = 0x00,
  Access         = 0x01,
  Connect        = 0x02,
  ExtendedAccess = 0x03,
  Named          = 0xFF,
};

enum AttributeModifier
{
  AttribNormal          = 0,
  AttribBytes           = 1,
  AttribRawBytes        = 2,
  AttribRawProcessBytes = 3,
};

enum AccessAttrib
{
  AttribQuick            = 0x02,
  AttribSendRecv         = 0x04,
  AttribByte             = 0x06,
  AttribWord             = 0x08,
  AttribBlock            = 0x0A,
  AttribProcessCall      = 0x0C,
  AttribBlockProcessCall = 0x0D
};

enum ExtAccessModifier
{
  ExtAttribBytes      = 0x0B,
  ExtAttribRawBytes   = 0x0E,
  ExtAttribRawProcess = 0x0F
};

typedef struct
{
  hash_key name;
  uint8_t  region_space;
  size_t   offset;
  size_t   len;
} aml_operation_region_t;

typedef struct
{
  enum AccessType         access_type;
  bool                    should_lock;
  enum UpdateRule         update_rule;
  aml_operation_region_t* region;
} aml_field_t;

typedef struct
{
  int          offset;
  int          len;
  hash_key     name;
  aml_field_t* parent;
} aml_named_field_t;

typedef struct
{
  uint8_t type;
  void*   data;
} aml_ptr_t;

typedef struct
{
  hash_key  name;
  uint16_t* field;
} aml_word_field_t;
