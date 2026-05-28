#include "namespace.h"

#include "hashmap.h"
#include "host.h"
#include "types.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void push_namespace(aml_namespace_t* parent, aml_namespace_t* this)
{
  hash_map_push(parent->namespaces, this->name, this, sizeof(aml_namespace_t));
}

aml_namespace_t* create_namespace(
  aml_namespace_t* parent,
  hash_key         name,
  uint8_t*         code,
  uint32_t         children,
  uint32_t         namespaces
)
{
  aml_namespace_t* namespace = malloc(sizeof(aml_namespace_t));
  if (!namespace) { return NULL; }
  namespace->parent = parent;
  namespace->code   = code;
  memcpy(namespace->name, name, KEY_LEN);
  namespace->children = hash_map_create(children ? children : 1);
  if (!namespace->children) { return NULL; }
  namespace->namespaces = hash_map_create(namespaces ? namespaces : 1);
  if (!namespace->namespaces) { return NULL; }
  if (parent) { push_namespace(parent, namespace); }
  return namespace;
}

aml_namespace_t* locate_namespace(aml_namespace_t* parent, aml_name_t key)
{
  aml_namespace_t* current = parent;
  aml_namespace_t* ns      = NULL;
  while (ns == NULL)
    {
      ns = hash_map_get(current->namespaces, key.inner, NULL);
      if (ns) { return ns; }
      current = current->parent;
      if (!current) { break; }
    }
  return NULL;
}

void add_child_to_namespace(aml_namespace_t* ns, hash_key key, aml_ptr_t* data)
{ hash_map_push(ns->children, key, data, sizeof(aml_ptr_t)); }

void debug_namespace(void* ptr)
{
  if (!ptr) { return; }
  aml_namespace_t* ns = ptr;
  printf("%s\n", ns->name);
  hash_map_debug(ns->children);
  hash_map_foreach(ns->namespaces, debug_namespace);
}

static aml_namespace_t* init_aml_namespaces()
{
  aml_namespace_t* root_ns = create_namespace(NULL, "\\___", NULL, 8, 4);
  if (!root_ns) { AML_EXIT(); }
  if (!create_namespace(root_ns, "_GPE", NULL, 0, 0)) { AML_EXIT(); }
  if (!create_namespace(root_ns, "_PR_", NULL, 0, 0)) { AML_EXIT(); }
  if (!create_namespace(root_ns, "_SB_", NULL, 8, 8)) { AML_EXIT(); }
  if (!create_namespace(root_ns, "_SI_", NULL, 0, 0)) { AML_EXIT(); }
  if (!create_namespace(root_ns, "_TZ_", NULL, 0, 0)) { AML_EXIT(); }
  return root_ns;
}

aml_namespace_t* root()
{
  static aml_namespace_t* _root = NULL;
  if (_root == NULL) { _root = init_aml_namespaces(); }
  return _root;
}

