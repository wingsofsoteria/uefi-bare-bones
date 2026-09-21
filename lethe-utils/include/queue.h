#pragma once

struct queue_t
{
  void** _inner;
  int    head;
  int    tail;
  int    capacity;
};

typedef struct queue_t queue_t;
void                   push_queue(queue_t* queue, void* task);
void*                  pop_queue(queue_t* queue);
queue_t*               new_queue();
