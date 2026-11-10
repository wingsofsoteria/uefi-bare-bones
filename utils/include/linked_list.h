#pragma once

typedef struct node
{
  void*        ptr;
  struct node* next;
  struct node* root;
} linked_list_node_t;

struct node* new_node();
struct node* append_node(struct node* list, void* ptr);
void         linked_list_foreach(struct node* list, void (*fn)(void*));
