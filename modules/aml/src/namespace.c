#include "namespace.h"

#include "hashmap.h"
#include "helpers.h"
#include "host.h"
#include "opcodes.h"
#include "stddef.h"
#include "types.h"

#include <stdio.h>
#include <string.h>

static void push_namespace(aml_namespace_t* parent, aml_namespace_t* this)
{ hm_set(parent->namespaces, this->name, this); }

aml_namespace_t* create_namespace(
  aml_namespace_t* parent,
  hash_key         name,
  uint8_t*         code
)
{
  aml_namespace_t* namespace = malloc(sizeof(aml_namespace_t));
  if (!namespace) { return NULL; }
  namespace->parent = parent;
  namespace->code   = code;
  memcpy(namespace->name, name, KEY_LEN);
  namespace->children = hm_create();
  if (!namespace->children) { return NULL; }
  namespace->namespaces = hm_create();
  if (!namespace->namespaces) { return NULL; }
  if (parent) { push_namespace(parent, namespace); }
  return namespace;
}

void add_child_to_namespace(aml_namespace_t* ns, hash_key key, aml_ptr_t* data)
{ hm_set(ns->children, key, data); }

void debug_ptr(const char* key, void* entry)
{
  aml_ptr_t* ptr = entry;
  switch (ptr->type)
    {
      case TYPE_NAME:
        {
          aml_variable_t* variable = ptr->data;
          valid_name(&(aml_name_t){ KEY_LEN, variable->label });
          aml_log("%s ", variable->label);
          switch (variable->data_type)
            {
              case DATA_BYTE:
                {
                  printf("%d\n", variable->byte);
                  break;
                }
              case DATA_BUF:
                {
                  aml_buffer_t* buf = variable->buffer;
                  for (int i = 0; i < buf->size; i++)
                    {
                      printf("%x ", buf->buffer[i]);
                    }
                  printf("\n");
                }
              case DATA_PKG:
                {
                  aml_package_t* pkg = variable->package;
                  for (int i = 0; i < pkg->num_elements; i++)
                    {
                      debug_ptr(NULL, pkg->elements[i]);
                    }
                }
              default:
                {
                  printf("\n");
                  break;
                }
            }
        }
      default:
        {
          break;
        }
    }
}

void debug_namespace(const char* key, void* ptr)
{
  if (!ptr) { return; }
  aml_namespace_t* ns = ptr;
  aml_log("%s\n", ns->name);
  hmi child_iter = hm_iter(ns->children);
  hm_foreach(&child_iter, debug_ptr);
  hmi ns_iter = hm_iter(ns->namespaces);

  hm_foreach(&ns_iter, debug_namespace);
}

static aml_namespace_t* init_aml_namespaces()
{
  aml_namespace_t* root_ns = create_namespace(NULL, "\\___", NULL);
  if (!root_ns) { AML_EXIT(); }
  if (!create_namespace(root_ns, "_GPE", NULL)) { AML_EXIT(); }
  if (!create_namespace(root_ns, "_PR_", NULL)) { AML_EXIT(); }
  if (!create_namespace(root_ns, "_SB_", NULL)) { AML_EXIT(); }
  if (!create_namespace(root_ns, "_SI_", NULL)) { AML_EXIT(); }
  if (!create_namespace(root_ns, "_TZ_", NULL)) { AML_EXIT(); }
  return root_ns;
}

aml_namespace_t* root()
{
  static aml_namespace_t* _root = NULL;
  if (_root == NULL) { _root = init_aml_namespaces(); }
  return _root;
}

