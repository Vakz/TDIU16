#include <stdio.h>
#include <syscall-nr.h>
#include "userprog/syscall.h"
#include "threads/interrupt.h"
#include "threads/thread.h"

/* header files you probably need, they are not used yet */
#include <string.h>
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "threads/vaddr.h"
#include "threads/init.h"
#include "userprog/pagedir.h"
#include "userprog/process.h"
#include "devices/input.h"

/* included for implementation of syscalls */
#include "threads/init.h"
#include "userprog/flist.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void)
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
  flist_init();
}


/* This array defined the number of arguments each syscall expects.
   For example, if you want to find out the number of arguments for
   the read system call you shall write:

   int sys_read_arg_count = argc[ SYS_READ ];

   All system calls have a name such as SYS_READ defined as an enum
   type, see `lib/syscall-nr.h'. Use them instead of numbers.
 */
const int argc[] = {
  /* basic calls */
  0, 1, 1, 1, 2, 1, 1, 1, 3, 3, 2, 1, 1,
  /* not implemented */
  2, 1,    1, 1, 2, 1, 1,
  /* extended */
  0
};

static int
syscall_read(int fd, void* buffer, unsigned int length) {
  char* buffer_p = (char*)buffer;
  if (fd == STDOUT_FILENO) return -1;

  // Reading from keyboard
  if (fd == STDIN_FILENO) {
    for (unsigned int i = 0; i < length; ++i) {
      buffer_p[i] = input_getc();
      if (buffer_p[i] == '\r') buffer_p[i] = '\n';
      putbuf(&buffer_p[i], 1);
    }
    return length;
  }
  // Reading a file
  struct file* f = flist_find(thread_current()->tid, fd);
  if (f == NULL) {

    return -1;
  }
  return file_read(f, buffer, length);
}

static int
syscall_write(int fd, const void* buffer, unsigned int length) {
  if (fd == STDIN_FILENO) return -1;

  // Writing to screen
  if (fd == STDOUT_FILENO) {
    putbuf((char*)buffer, length);
    return length;
  }

  // Writing to file
  struct file* f = flist_find(thread_current()->tid, fd);
  if (f == NULL) return -1;
  return file_write(f, buffer, length);
}

static int
syscall_open(const char* file) {
  struct file* opened_file = filesys_open(file);
  if (opened_file == NULL) {
    return -1;
  }
  return flist_insert(opened_file, thread_current()->tid);
}

static void
syscall_seek(int fd, unsigned int position) {
  struct file* f = flist_find(thread_current()->tid, fd);
  if (f != NULL) {
    file_seek(f, position);
  }
}

static unsigned int
syscall_tell(int fd) {
  struct file* f = flist_find(thread_current()->tid, fd);
  if (f != NULL) {
    return file_tell(f);
  }
  return -1;
}

static int
syscall_filesize(int fd) {
  struct file* f = flist_find(thread_current()->tid, fd);
  if (f != NULL) {
    return file_length(f);
  }
  return -1;
}

static void
syscall_handler (struct intr_frame *f)
{
  int32_t* esp = (int32_t*)f->esp;
  switch ( *esp )
  {
    case SYS_HALT:
    {
      power_off();
    }
    case SYS_EXIT:
    {
      int status = *(esp+1);
      printf("Status code: %i\n", status);
      process_exit(status);
      thread_exit();
      break;
    }
    case SYS_READ:
    {
      // fd, buffer, length
      f->eax = syscall_read(esp[1], (void*)esp[2], esp[3]);
      break;
    }
    case SYS_WRITE:
    {
      // fd, buffer, length
      f->eax = syscall_write(esp[1], (void*)esp[2], esp[3]);
      break;
    }
    case SYS_OPEN:
    {
      f->eax = syscall_open((char*)esp[1]);
      break;
    }
    case SYS_REMOVE:
    {

      f->eax = filesys_remove((char*)esp[1]);
      break;
    }
    case SYS_CREATE:
    {
      f->eax = filesys_create((char*)esp[1], esp[2]);
      break;
    }
    case SYS_CLOSE:
    {
      flist_remove(thread_current()->tid, esp[1]);
      break;
    }
    case SYS_SEEK:
    {
      syscall_seek(esp[1], esp[2]);
      break;
    }
    case SYS_TELL:
    {
      f->eax = syscall_tell(esp[1]);
      break;
    }
    case SYS_FILESIZE:
    {
      f->eax = syscall_filesize(esp[1]);
      break;
    }
    default:
    {
      printf ("Executed an unknown system call!\n");

      printf ("Stack top + 0: %d\n", esp[0]);
      printf ("Stack top + 1: %d\n", esp[1]);

      thread_exit ();
    }
  }
}
