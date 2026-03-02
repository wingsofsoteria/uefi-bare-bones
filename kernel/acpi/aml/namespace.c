#include "aml.h"
#include "stdlib.h"
aml_node_t* aml_root = NULL;

aml_node_t* aml_root_node()
{
  return aml_root;
}

void aml_append_node(aml_node_t* parent, aml_node_t* this)
{
  this->parent  = parent;
  this->child   = parent->child;
  parent->child = this;
}

aml_node_t* aml_create_node()
{
  aml_node_t* node = calloc(1, sizeof(aml_node_t));
  node->name       = NULL;
  node->child      = NULL;
  node->parent     = NULL;
  node->data       = (aml_ptr_t){0, NULL};
  return node;
}

void aml_node_init()
{
  aml_root = aml_create_node();
}
