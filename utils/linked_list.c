#include "linked_list.h"

#include "stddef.h"
#include "stdint.h"

#include <stdio.h>

#define DEBUG
extern void* calloc(size_t, size_t);

struct node* new_node()
{
  struct node* node = calloc(1, sizeof(struct node));
  node->next        = NULL;
  node->ptr         = NULL;
  node->root        = node;
  return node;
}

struct node* append_node(struct node* list, void* ptr)
{
  struct node* node = new_node();
  node->next        = list->next;
  list->next        = node;
  node->ptr         = ptr;
  node->root        = list;
  return node;
}

void linked_list_foreach(struct node* list, void (*fn)(void*))
{

#ifdef DEBUG
  printf("Iterating list %p\n", list);
  int i = 0;
#endif
  for (struct node* node = list; node != NULL; node = node->next)
    {
#ifdef DEBUG
      printf(
        "Iteration %d executing function on node %p %p %p\n",
        i++,
        node,
        node->ptr,
        node->next
      );
#endif
      fn(node->ptr);
    }
#ifdef DEBUG
  printf("Iteration ended after %d loops\n", i);
#endif
}
