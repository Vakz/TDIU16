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

struct flist file_list;

void
syscall_init (void)
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
  flist_init(&file_list);
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

static void
read_keyboard(char* buffer, unsigned int length) {
  for (unsigned int i = 0; i < length; ++i) {
    buffer[i] = input_getc();
    if (buffer[i] == '\r') buffer[i] = '\n';
    putbuf(&buffer[i], 1);
  }
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
      int fd = esp[1];
      char* buffer = (char*)esp[2];
      unsigned int length = esp[3];

      if (fd != STDIN_FILENO) {
        f->eax = -1;
        break;
      }
      else {
        read_keyboard(buffer, length);
      }

      f->eax = esp[3];
      break;
    }
    case SYS_WRITE:
    {
      int fd = esp[1];

      if (fd != STDOUT_FILENO) {
        f->eax = -1;
        break;
      }

      putbuf((char*)esp[2], esp[3]);
      f->eax = esp[3];
      break;
    }
    case SYS_OPEN:
    {
      const char* filename = (char*)esp[1];
      struct file* opened_file = filesys_open(filename);
      if (opened_file == NULL) {
        f->eax = -1;
      }
      flist_insert(&file_list, opened_file, thread_current()->tid);
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
