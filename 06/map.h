#define PANIC() exit(1)

#include "list.h"

typedef char* value_t;
typedef int key_t;

struct map_element {
  struct list_elem elem;
  value_t value;
  key_t key;
};

struct map
{
  struct list content;
  int next_key;
};

void map_init(struct map* m);
key_t map_insert(struct map* m, value_t v);
struct map_element* find_list_elem(struct list* l, key_t k);
value_t map_find(struct map* m, key_t k);
value_t map_remove(struct map* m, key_t k);
void map_for_each(struct map* m, void (*exec)(key_t k, value_t v, int aux), int aux);
void map_remove_if(struct map* m, bool (*cond)(key_t k, value_t v, int aux), int aux);
