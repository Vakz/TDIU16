#include <stddef.h>

#include "plist.h"

struct process_elem listed_processes[PLIST_SIZE];

void plist_init(void){
  for (int i = 0; i < PLIST_SIZE; ++i) {
    listed_processes[i].used = false;
    lock_init(&listed_processes[i].in_use);
  }
}

int plist_insert(int parent_id, int process_id){
  bool parent_exists = parent_id == 1;
  int first_free = -1;
  for (int i = 0; i < PLIST_SIZE; ++i) {
    if (listed_processes[i].used && listed_processes[i].proc_id == process_id) {
      return -1;
    }
    if (listed_processes[i].used && listed_processes[i].proc_id == parent_id) {
      parent_exists = true;
    }
    else if (first_free == -1 && !listed_processes[i].used && lock_try_acquire(&listed_processes[i].in_use) ) {
      first_free = i;
    }
  }
  if (!parent_exists || first_free == -1) {
    return -1;
  }

  struct process_elem* p = &listed_processes[first_free];

  //skapa plist element
  p->proc_id = process_id;
  p->parent_id = parent_id;
  p->exit_status = -1;
  p->alive = true;
  p->parent_alive = true;
  p->used = true;
  sema_init(&p->exit_sync, 0);
  lock_release(&p->in_use);
  return process_id;
}

struct process_elem* plist_find(int process_id) {
  for (int i = 0; i < PLIST_SIZE; ++i){
    if(listed_processes[i].used && listed_processes[i].proc_id == process_id){
      return &listed_processes[i];
    }
  }
  return NULL;
}


void plist_remove(int process_id){
  struct process_elem* p;
  for(int i=0; i<PLIST_SIZE; ++i){
    p = &listed_processes[i];
    if(p->used && p->proc_id == process_id){
      p->alive = false;
      if(!p->parent_alive){
        p->used = false;
      }
    }
    else if(p->used && p->parent_id == process_id){
      p->parent_alive = false;
      if(!p->alive){
        p->used = false;
      }
    }
  }
}

void plist_for_each(void (*exec)(struct process_elem* p)){
  for(int i=0; i<PLIST_SIZE; ++i){
    if(listed_processes[i].used){
      exec(&listed_processes[i]);
    }
  }
}
