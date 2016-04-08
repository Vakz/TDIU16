#include "list.h"
#include <stdio.h>
#include <stdlib.h>

#define N 100

struct our_struct {
  int value;
  struct list_elem elem;
};

struct list my_list;


int main() {
  list_init(&my_list);
  for (int i = 2; i < N; ++i) {
    struct our_struct* elem = (struct our_struct*)malloc(sizeof(struct our_struct));
    elem->value = i;
    list_push_front(&my_list, &elem->elem);
  }

  for (int i = 2; i < N; ++i) {
    for (int j = i*2; j < N; j += i) {

      for (struct list_elem *e = list_begin(&my_list); e != list_end(&my_list);) {
        struct our_struct* elem = list_entry(e, struct our_struct, elem);
        if (elem->value % j == 0) {
          e = list_remove(e);
          free(elem);
        }
        else {
          e = list_next(e);
        }
      }
    }

    for (struct list_elem *e = list_begin(&my_list); e != list_end(&my_list);) {
      struct our_struct* elem = list_entry(e, struct our_struct, elem);
      printf("%i\n", elem->value);
      e = list_remove(e);
      free(elem);
    }
  }
}
