#include "map.h"
#include "list.h"
#include <stdlib.h>
#include <stdio.h>

void map_init(struct map* m) {
  list_init(&m->content);
}

key_t map_insert(struct map* m, value_t v) {
  struct map_element* elem = (struct map_element*)malloc(sizeof(struct map_element));
  elem->value = v;
  elem->key = m->next_key++;

  list_push_front(&m->content, &elem->elem);
}

struct map_element* find_list_elem(struct list* l, key_t k) {
  for (struct list_elem* e = list_begin(l); e != list_end(l); e = list_next(e)) {
    struct map_element* elem = list_entry(e, struct map_element, elem);
    if (elem->key == k) {
      return elem;
    }
  }
  return NULL;
}

value_t map_find(struct map* m, key_t k) {
  struct map_element* e = find_list_elem(&m->content, k);
  printf("Returning %i", e->value);
  return e == NULL ? NULL : e->value;
}

value_t map_remove(struct map* m, key_t k) {
  struct map_element* e = find_list_elem(&m->content, k);
  if (e == NULL) return NULL;
  value_t v = e->value;
  list_remove(&e->elem);
  free(e);
  return v;
}

void map_for_each(struct map* m, void (*exec)(key_t k, value_t v, int aux), int aux) {
  for (struct list_elem* e = list_begin(&m->content); e != list_end(&m->content); e = list_next(e)) {
    struct map_element* elem = list_entry(e, struct map_element, elem);
    exec(elem->key, elem->value, aux);
  }
}

void map_remove_if(struct map* m, bool (*cond)(key_t k, value_t v, int aux), int aux) {
  for (struct list_elem*e = list_begin(&m->content); e != list_end(&m->content);) {
    struct map_element* elem = list_entry(e, struct map_element, elem);
    if (cond(elem->key, elem->value, aux)) {
      e = list_remove(e);
      free(e);
    }
    else {
      e = list_next(e);
    }
  }
}
