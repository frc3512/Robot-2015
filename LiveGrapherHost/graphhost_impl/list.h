#ifndef _LIST_H
#define _LIST_H

struct list_elem_t {
  void *data;

  struct list_elem_t *next;
  struct list_elem_t *prev;
};

struct list_t {
  struct list_elem_t *start;
};

struct list_t * list_create();
void list_destroy(struct list_t *list);
void list_delete(struct list_t *list, struct list_elem_t *elem);

struct list_elem_t *
list_add_after(struct list_t *list, struct list_elem_t *before,
  void *data);

#endif

