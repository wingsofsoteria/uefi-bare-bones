#include "namespace.h"

#include "hashmap.h"
#include "helpers.h"
#include "host.h"
#include "name.h"
#include "types.h"

#include <assert.h>
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
  alog("\n");
  aml_namespace_t* namespace = malloc(sizeof(aml_namespace_t));
  assert(namespace != NULL);
  namespace->parent = parent;
  namespace->code   = code;
  memcpy(namespace->name, name, KEY_LEN);
  namespace->children = hash_map_create(children ? children : 1);
  assert(namespace->children != NULL);
  namespace->namespaces = hash_map_create(namespaces ? namespaces : 1);
  assert(namespace->namespaces != NULL);
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
{
  alog("\n");
  // aml_name_t resolved_name = resolve_name(ns, key);
  /*if (key.inner[0] == '^')
    {
      data_parent = ns->parent;
      for (int i = 0; i < key.count; i++) { key.inner[i] = key.inner[i + 1]; }
      key.inner[key.count] = 0;
    }
  else if (key.inner[0] == '\\')
    {
      alog("%s %s\n", ns->name, key.inner);
      debug_exit();
      aml_namespace_t* __unused = get_scope(data_parent, key);
      assert(!__unused);

      key = trim_name(&key);
      alog("%s %s\n", data_parent->name, key.inner);
    }*/
  hash_map_push(ns->children, key, data, sizeof(aml_ptr_t));
}

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
  alog("\n");
  aml_namespace_t* root_ns = create_namespace(NULL, "\\___", NULL, 8, 4);
  aml_namespace_t* gpe_ns  = create_namespace(root_ns, "_GPE", NULL, 0, 0);
  aml_namespace_t* pr_ns   = create_namespace(root_ns, "_PR_", NULL, 0, 0);
  aml_namespace_t* sb_ns   = create_namespace(root_ns, "_SB_", NULL, 8, 8);
  aml_namespace_t* si_ns   = create_namespace(root_ns, "_SI_", NULL, 0, 0);
  aml_namespace_t* tz_ns   = create_namespace(root_ns, "_TZ_", NULL, 0, 0);
  return root_ns;
}

aml_namespace_t* root()
{
  static aml_namespace_t* _root = NULL;
  if (_root == NULL) { _root = init_aml_namespaces(); }
  return _root;
}

