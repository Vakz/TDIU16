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
#include "userprog/plist.h"
#include "devices/timer.h"

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

static bool
syscall_verify_variable_length(char* start)
{
  unsigned current_page = pg_no(start);
  if (pagedir_get_page(thread_current()->pagedir, start) == NULL)
    return false;
  for (int i = 0; ; ++i) {
    char* address = start+i;
    if (current_page != pg_no(address)) {
      current_page = pg_no(address);
      if (pagedir_get_page(thread_current()->pagedir, address) == NULL)
        return false;
    }
    if (*address == '\0') return true;
  }
}

static bool
syscall_verify_fix_length(void* start, int length)
{
  void* end = (void*)((char*)start + length);
  if (is_kernel_vaddr(end)) return false;
  for (void* i = pg_round_down(start); i < end; i = (void*)((char*)i + PGSIZE)) {

    if (pagedir_get_page(thread_current()->pagedir, i) == NULL) return false;
  }
  return true;
}

static bool
syscall_verify_fix_pointer(void* p, int length) {
  if (p == NULL) return false;
  if (is_kernel_vaddr(p)) return false;
  return syscall_verify_fix_length(p, length);
}

static bool
syscall_verify_pointer(char* p) {
  if (p == NULL) return false;
  if (is_kernel_vaddr(p)) return false;
  return syscall_verify_variable_length(p);
}

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
  if (!syscall_verify_fix_pointer((void*)esp, 16)) {
    thread_exit();
  }
  switch ( *esp )
  {
    case SYS_HALT:
    {
      power_off();
    }
    case SYS_EXIT:
    {

      int status = *(esp+1);
      process_exit(status);
      thread_exit();
      break;
    }
    case SYS_EXEC:
    {
      char* name = (char*)esp[1];
      if (!syscall_verify_pointer(name)) {
        thread_exit();
      }
      else {
        f->eax = process_execute(name);
      }

      break;
    }
    case SYS_READ:
    {
      // fd, buffer, length
      void* buf = (void*)esp[2];
      if (!syscall_verify_fix_pointer(buf, esp[3])) {
        thread_exit();
      }
      else {
        f->eax = syscall_read(esp[1], buf, esp[3]);
      }
      break;
    }
    case SYS_WRITE:
    {
      // fd, buffer, length
      void* buf = (void*)esp[2];
      if (!syscall_verify_fix_pointer(buf, esp[3])) {
        thread_exit();
      }
      else {
        f->eax = syscall_write(esp[1], buf, esp[3]);
      }
      break;
    }
    case SYS_OPEN:
    {
      char* name = (char*)esp[1];
      if (!syscall_verify_pointer(name)) {
        thread_exit();
      }
      else {
        f->eax = syscall_open(name);
      }
      break;
    }
    case SYS_REMOVE:
    {
      char* name = (char*)esp[1];
      if (!syscall_verify_pointer(name)) {
        thread_exit();
      }
      else {
        f->eax = filesys_remove(name);
      }
      break;
    }
    case SYS_CREATE:
    {
      char* name = (char*)esp[1];
      if (!syscall_verify_pointer(name)) {
        thread_exit();
      }
      else {
        f->eax = filesys_create(name, esp[2]);
      }
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
    case SYS_SLEEP:
    {
      timer_msleep(esp[1]);
      break;
    }
    case SYS_PLIST:
    {
      process_print_list();
      break;
    }
    case SYS_WAIT:
    {

      f->eax = process_wait(esp[1]);
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
