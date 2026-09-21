#include "queue.h"

#include "stddef.h"

#include <assert.h>

#define TASK_QUEUE_SIZE 10
extern void* malloc(size_t);

void push_queue(struct queue_t* queue, void* task)
{
  if (((queue->head + 1) % queue->capacity) == queue->tail)
    {
      return; // queue is full
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

struct queue_t* new_queue()
{
  struct queue_t* queue = malloc(sizeof(queue_t));
  assert(queue != NULL);
  queue->head     = 0;
  queue->tail     = 0;
  queue->capacity = TASK_QUEUE_SIZE;
  queue->_inner   = malloc(queue->capacity * sizeof(void*));

  return queue;
}
