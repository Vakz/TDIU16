#include <stddef.h>
#include <stdlib.h>
#include "threads/malloc.h"
#include <list.h>
#include <stdio.h>
#include "flist.h"

struct flist open_files;

/* Completely lacking of handling when pid != flist_element->pid. Now just does nothing */

void flist_init(void) {
  open_files.next_key = 2;
  list_init(&open_files.content);
}

int flist_insert(struct file* f, int pid) {
  struct flist_element* elem = (struct flist_element*)malloc(sizeof(struct flist_element));
  elem->f = f;
  elem->pid = pid;
  elem->fd = open_files.next_key++;

  list_push_back(&open_files.content, &elem->elem);
  return elem->fd;
}

struct flist_element* flist_find_elem(int fd) {
  for (struct list_elem* e = list_begin(&open_files.content); e != list_end(&open_files.content); e = list_next(e)) {
    struct flist_element* elem = list_entry(e, struct flist_element, elem);
    if (elem->fd == fd) {
      return elem;
    }
  }
  return NULL;
}

struct file* flist_find(int pid, int fd) {
  struct flist_element* e = flist_find_elem(fd);
  return e == NULL || e->pid != pid ? NULL : e->f;
}

void flist_free_element(struct flist_element* e) {
  file_close(e->f);
  free(e);
}

void flist_remove(int pid, int fd) {
  struct flist_element* e = flist_find_elem(fd);
  //if (e == NULL || e->pid != pid) Do something
  if (e != NULL && e->pid == pid) {
    list_remove(&e->elem);
    flist_free_element(e);
  }
}

void flist_for_each(int pid, void (*exec)(int fd, struct file* f, int aux), int aux) {
  for (struct list_elem* e = list_begin(&open_files.content); e != list_end(&open_files.content); e = list_next(e)) {
    struct flist_element* elem = list_entry(e, struct flist_element, elem);
    if (elem->pid == pid) exec(elem->pid, elem->f, aux);
  }
}

void flist_remove_if(int pid, bool (*cond)(int fd, struct file* f, int aux), int aux) {
  for (struct list_elem* e = list_begin(&open_files.content); e != list_end(&open_files.content);) {
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

void flist_close_process_files(int pid) {
  for (struct list_elem* e = list_begin(&open_files.content); e != list_end(&open_files.content);) {
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
