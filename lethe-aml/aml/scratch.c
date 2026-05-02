/*
 * method: (name, code) should be stored within a namespace (scope, device, etc)
 * */

#include "aml.h"

typedef struct aml_namespace
{
  struct aml_namespace* parent;
  char*                 name;
  uint8_t*              code;
  void**                children;
} aml_namespace_t;

static aml_namespace_t* _root = NULL;

static aml_namespace_t* create_namespace(
  aml_namespace_t* parent,
  char*            name,
  uint8_t*         code
)
{
  aml_namespace_t* namespace = malloc(sizeof(aml_namespace_t));
  if (!namespace) return NULL;
  namespace->parent = parent;
  namespace->code   = code;
  namespace->name   = name;
  return namespace;
}

static int init_aml_namespaces(uint8_t* root_code)
{
  aml_namespace_t* root_ns = create_namespace(NULL, "\\___", root_code);
  if (!root_ns) return 1;
  _root = root_ns;
  return 0;
}

static void parse_aml(aml_namespace_t* parent, uint8_t* code) {
  for (uint8_t op = *code; code != NULL; code++) {
    switch (op) {

      case IF_OP: {

        break;
      }
    }
  }
  printf("%x", *code);
}

static void populate_children(aml_namespace_t* ns) { parse_aml(ns, ns->code); }

void parse_table(acpi_aml_table_t* table)
{
  if (!_root) {
    if (init_aml_namespaces(table->definition_blocks)) {return;}
  }
  populate_children(_root);
}
