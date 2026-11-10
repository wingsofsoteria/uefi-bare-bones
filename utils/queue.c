#include "queue.h"

#include "stddef.h"

#include <assert.h>

#define TASK_QUEUE_SIZE 10
#define RESIZE_VAL      2
extern void* malloc(size_t);
extern void* realloc(void*, size_t);

void push_queue(struct queue_t* queue, void* task)
{
  if (((queue->head + 1) % queue->capacity) == queue->tail)
    {
      if (!queue->resizable) return; // queue is full

      int capacity  = queue->capacity * RESIZE_VAL;
      queue->_inner = realloc(queue->_inner, capacity);
      assert(queue->_inner != NULL);
      queue->capacity = capacity;
      queue->tail     = (queue->tail + 1) % queue->capacity;
    }
  queue->_inner[queue->tail] = task;
  queue->tail                = (queue->tail + 1) % queue->capacity;
}

void* pop_queue(struct queue_t* queue)
{
  if (queue->head == queue->tail) { return NULL; }
  void* ptr   = queue->_inner[queue->head];
  queue->head = (queue->head + 1) % queue->capacity;
  if (ptr == NULL) { return pop_queue(queue); }
  return ptr;
}

void* rotate_queue(struct queue_t* queue)
{
  void* ptr = pop_queue(queue);
  push_queue(queue, ptr);
  return ptr;
}

struct queue_t* new_queue()
{
  struct queue_t* queue = malloc(sizeof(queue_t));
  assert(queue != NULL);
  queue->head      = 0;
  queue->tail      = 0;
  queue->capacity  = TASK_QUEUE_SIZE;
  queue->_inner    = malloc(queue->capacity * sizeof(void*));
  queue->resizable = false;
  return queue;
}

struct queue_t* new_queue_resizable()
{
  struct queue_t* queue = new_queue();
  queue->resizable      = true;
  return queue;
}
