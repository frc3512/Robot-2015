#include <stdlib.h>
#include "list.h"

struct list_t *
list_create()
{
  struct list_t *list;

  list = malloc(sizeof(struct list_t));
  list->start = NULL;

  return list;
}

void
list_destroy(struct list_t *list)
{
  struct list_elem_t *elem;
  struct list_elem_t *last = NULL;

  for(elem = list->start; elem != NULL; elem = elem->next) {
    free(last);
    last = elem;
  }
  free(last);

  /* Free the whole list handle */
  free(list);

  return;
}

void
list_delete(struct list_t *list, struct list_elem_t *elem)
{
  if(elem == NULL) return;

  if(list->start == elem)
    list->start = elem->next;

  if(elem->next != NULL) {
    elem->next->prev = elem->prev;
  }

  if(elem->prev != NULL) {
    elem->prev->next = elem->next;
  }

  free(elem);

  return;
}

struct list_elem_t *
list_add_after(struct list_t *list, struct list_elem_t *before, void *data)
{
  struct list_elem_t *elem;

  /* Allocate a new element */
  elem = malloc(sizeof(struct list_elem_t));

  /* Insert the data into it */
  elem->data = data;

  /* Insert the element into the list */
  if(before == NULL) {
    /* If before is null this will be the first element
       in the list. */
    elem->next = list->start;
    elem->prev = NULL;
    if(list->start != NULL)
      list->start->prev = elem;
    list->start = elem;
  }else{
    /* If before is not null, just add this element after
       before. */
    elem->next = before->next;
    elem->prev = before;
    before->next->prev = elem;
    before->next = elem;
  }

  return elem;
}

