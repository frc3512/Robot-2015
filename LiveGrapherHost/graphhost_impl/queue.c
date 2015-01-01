#include <stdlib.h>
#include "queue.h"

struct queue_t *
queue_init(int size)
{
  struct queue_t *queue;

  /* Create the queue description structure */
  queue = malloc(sizeof(struct queue_t ));

  queue->list = list_create();
  queue->last = NULL;
  queue->size = size;
  queue->length = 0;

  return queue;
}

int
queue_dequeue(struct queue_t *queue, void **data)
{
  struct list_elem_t *elem;

  if(queue->last == NULL) return -1;

  *data = queue->last->data;
  elem = queue->last;
  queue->last = elem->prev;
  list_delete(queue->list, elem);
  queue->length--;

  return 0;
}

int
queue_queue(struct queue_t *queue, void *data)
{
  struct list_elem_t *elem;

  if(queue->length >= queue->size) return -1;

  elem = list_add_after(queue->list, NULL, data);
  queue->length++;

  if(queue->last == NULL) {
    queue->last = elem;
  }

  return 0;
}

void
queue_free(struct queue_t *queue)
{
  list_destroy(queue->list);
  free(queue);

  return;
}
