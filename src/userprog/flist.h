#ifndef _flist_H_
#define _flist_H_

/* Place functions to handle a process open files here (file list).

   flist.h : Your function declarations and documentation.
   flist.c : Your implementation.

   The following is strongly recommended:

   - A function that given a file (struct file*, see filesys/file.h)
     and a process id INSERT this in a list of files. Return an
     integer that can be used to find the opened file later.

   - A function that given an integer (obtained from above function)
     and a process id FIND the file in a list. Should return NULL if
     the specified process did not insert the file or already removed
     it.

   - A function that given an integer (obtained from above function)
     and a process id REMOVE the file from a list. Should return NULL
     if the specified process did not insert the file or already
     removed it.

   - A function that given a process id REMOVE ALL files the specified
     process have in the list.

   All files obtained from filesys/filesys.c:filesys_open() are
   considered OPEN files and must be added to a list or else kept
   track of, to guarantee ALL open files are eventyally CLOSED
   (probably when removed from the list(s)).
 */

#include "filesys/file.h"

struct flist
{
  struct list content;
  int next_key;
};

struct flist_element {
  struct list_elem elem;
  struct file* f;
  int fd;
  int pid;
};


void flist_init(void);
int flist_insert(struct file* f, int pid);
struct file* flist_find(int pid, int fd);
struct flist_element* flist_find_elem(int fd);
void flist_free_element(struct flist_element* e);
void flist_remove(int pid, int fd);
void flist_for_each(int pid, void (*exec)(int fd, struct file* f, int aux), int aux);
void flist_remove_if(int pid, bool (*cond)(int fd, struct file* f, int aux), int aux);
void flist_close_process_files(int pid);


#endif
