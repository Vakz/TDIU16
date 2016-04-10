#include <stddef.h>
#include <stdlib.h>
#include "threads/malloc.h"
#include <list.h>
#include <stdio.h>
#include "flist.h"

/* Completely lacking of handling when pid != flist_element->pid. Now just does nothing */

void flist_init(struct flist* m) {
  m->next_key = 0;
  list_init(&m->content);
}

int flist_insert(struct flist* m, struct file* f, int pid) {
  struct flist_element* elem = (struct flist_element*)malloc(sizeof(struct flist_element));
  elem->f = f;
  elem->pid = pid;
  elem->fd = m->next_key++;

  list_push_front(&m->content, &elem->elem);
  return elem->fd;
}

struct flist_element* find_flist_elem(struct list* l, int fd) {
  for (struct list_elem* e = list_begin(l); e != list_end(l); e = list_next(e)) {
    struct flist_element* elem = list_entry(e, struct flist_element, elem);
    if (elem->fd == fd) {
      return elem;
    }
  }
  return NULL;
}

struct file* flist_find(struct flist* m, int pid, int fd) {
  struct flist_element* e = find_flist_elem(&m->content, fd);
  return e == NULL || e->pid != pid ? NULL : e->f;
}

void flist_free_element(struct flist_element* e) {
  file_close(e->f);
  free(e);
}

void flist_remove(struct flist* m, int pid, int fd) {
  struct flist_element* e = find_flist_elem(&m->content, fd);
  //if (e == NULL || e->pid != pid) Do something
  if (e != NULL && e->pid == pid) flist_free_element(e);
}

void flist_for_each(struct flist* m, int pid, void (*exec)(int fd, struct file* f, int aux), int aux) {
  for (struct list_elem* e = list_begin(&m->content); e != list_end(&m->content); e = list_next(e)) {
    struct flist_element* elem = list_entry(e, struct flist_element, elem);
    if (elem->pid == pid) exec(elem->pid, elem->f, aux);
  }
}

void flist_remove_if(struct flist* m, int pid, bool (*cond)(int fd, struct file* f, int aux), int aux) {
  for (struct list_elem* e = list_begin(&m->content); e != list_end(&m->content);) {
    struct flist_element* elem = list_entry(e, struct flist_element, elem);
    if (elem->pid == pid && cond(elem->pid, elem->f, aux)) {
      e = list_remove(e);
      flist_free_element(elem);
    }
    else {
      e = list_next(e);
    }
  }
}

void flist_close_process_files(struct flist* l, int pid) {
  for (struct list_elem* e = list_begin(&l->content); e != list_end(&l->content);) {
    struct flist_element* elem = list_entry(e, struct flist_element, elem);
    if (elem->pid == pid) {
      e = list_remove(e);
      flist_free_element(elem);
    }
    else {
      e = list_next(e);
    }
  }
}
