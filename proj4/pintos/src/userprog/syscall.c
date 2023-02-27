#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "lib/string.h"
#include "devices/shutdown.h"
#include "threads/vaddr.h"
#include "userprog/process.h"
#include "lib/stdio.h"
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "devices/input.h"
#include "userprog/pagedir.h"
#include "userprog/pagedir.h"
#include "threads/pte.h"

static void syscall_handler (struct intr_frame *);

struct lock access_lock;

void
syscall_init (void) 
{
  lock_init(&access_lock);
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}
bool valid_access(const void* p);
void address_check(const void* p){
  if(!valid_access(p))
    exit(-1);
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  uint32_t callnum = *(uint32_t*)f->esp;
  //hex_dump((uintptr_t)f->esp, f->esp, PHYS_BASE - f->esp, true);
  switch(callnum){
    case SYS_HALT: 
	halt(); break;
    case SYS_EXIT:
	address_check(f->esp + 4);
	exit(*(int*)(f->esp + 4)); break;
    case SYS_EXEC:
        address_check(f->esp + 4);
	f->eax = exec(*(char**)(f->esp + 4)); break;
    case SYS_WAIT:
        address_check(f->esp + 4);
	f->eax = wait(*(tid_t*)(f->esp + 4)); break;
    case SYS_CREATE:
        address_check(f->esp + 4);
        address_check(f->esp + 8);
	f->eax = create(*(char**)(f->esp + 4),*(unsigned*)(f->esp + 8));
	break;
    case SYS_REMOVE:
        address_check(f->esp + 4);
	f->eax = remove(*(char**)(f->esp + 4));
	break;
    case SYS_OPEN:
        address_check(f->esp + 4);
	f->eax = open(*(char**)(f->esp + 4));
	break;
    case SYS_CLOSE:
        address_check(f->esp + 4);
	close(*(int*)(f->esp + 4));
	break;
    case SYS_FILESIZE:
        address_check(f->esp + 4);
	f->eax = filesize(*(int*)(f->esp + 4));
	break;
    case SYS_SEEK:
        address_check(f->esp + 4);
        address_check(f->esp + 8);
	seek(*(int*)(f->esp + 4),*(unsigned*)(f->esp + 8));
	break;
    case SYS_TELL:
        address_check(f->esp + 4);
	f->eax = tell(*(int*)(f->esp + 4));
	break;
    case SYS_READ:
        address_check(f->esp + 4);
        address_check(f->esp + 8);
        address_check(f->esp + 12);
        f->eax = read(*(int*)(f->esp + 4), *(char**)(f->esp + 8), *(unsigned*)(f->esp + 12)); 
	break;
    case SYS_WRITE:
        address_check(f->esp + 4);
        address_check(f->esp + 8);
        address_check(f->esp + 12);
	f->eax = write(*(int*)(f->esp + 4), *(char**)(f->esp + 8), *(unsigned*)(f->esp + 12));
       	break;
    case SYS_FIBO:
        address_check(f->esp + 4);
	f->eax = fibonacci(*(int*)(f->esp + 4));
	break;
    case SYS_MAXF:
        address_check(f->esp + 4);
        address_check(f->esp + 8);
        address_check(f->esp + 12);
        address_check(f->esp + 16);
	 f->eax = max_of_four_int(*(int*)(f->esp + 4), *(int*)(f->esp + 8), 
			 *(int*)(f->esp + 12), *(int*)(f->esp + 16));
	 break;
  }


}

static uint32_t *
lookup_page (uint32_t *pd, const void *vaddr, bool create)
{
  uint32_t *pt, *pde;

  ASSERT (pd != NULL);

  /* Shouldn't create new kernel virtual mappings. */
  ASSERT (!create || is_user_vaddr (vaddr));

  /* Check for a page table for VADDR.
     If one is missing, create one if requested. */
  pde = pd + pd_no (vaddr);
  if (*pde == 0)
    {
      if (create)
        {
          pt = palloc_get_page (PAL_ZERO);
          if (pt == NULL)
            return NULL;

          *pde = pde_create (pt);
        }
      else
        return NULL;
    }

  /* Return the page table entry. */
  pt = pde_get_pt (*pde);
  return &pt[pt_no (vaddr)];
}


bool valid_access(const void* p){
  struct thread* cur = thread_current();
  if(p == NULL || is_kernel_vaddr(p) ||
		  lookup_page(cur->pagedir, p, false) == NULL)
	  return false;

  return true;
}

void halt(void){
  shutdown_power_off();
}

void exit(int status){
  struct thread *cur = thread_current();
  char *name = cur->name;
  cur->exit_status = status;
  printf("%s: exit(%d)\n", name, status);

  thread_exit();
}

tid_t exec(const char *cmd_line){
  address_check(cmd_line);
  char file_name[128];
  char *delimp;
  if((delimp = strchr(cmd_line, ' ')) == NULL)
    strlcpy(file_name, cmd_line, strlen(cmd_line) + 1);
  else
    strlcpy(file_name ,cmd_line, delimp - cmd_line + 1);

  if(filesys_open(file_name) == NULL){
    return -1;
  }
  
  tid_t tid = process_execute(cmd_line);
  
  return tid;
}

int wait(tid_t pid){
  return process_wait(pid);
}

int read(int fd, void *buffer, unsigned size)
{
  address_check(buffer);
  lock_acquire(&access_lock);
  int ret = -1;
  if(fd == 0){
    for(unsigned i=0;i<size;i++){
      *((int*)buffer + i) = input_getc();
      if(*((int*)buffer + i) == '\0')
	ret = i;
    }
  }
  else if(fd > 2){
    struct file *fp = thread_current()->fd_table[fd];
    if(fp != NULL) {
      ret = file_read(fp, buffer, size);
    }
  }
  lock_release(&access_lock);
  return ret;
}

int write(int fd, const void *buffer, unsigned size){  
  address_check(buffer);
  lock_acquire(&access_lock);
  int ret = -1;
  if(fd == 1){
    putbuf((char*)buffer, size);
    ret = size;
  }
  else if(fd > 2){
    struct file *fp = thread_current()->fd_table[fd];
    if(fp != NULL){
      ret = file_write(fp, buffer, size);
    }
  }
  lock_release(&access_lock);
  return ret;
}

int fibonacci(int n){
  int cur = 1;
  int before = 0;
  int ret = 1;
  for(int i=1;i<n;i++){
    ret = cur + before;
    before = cur;
    cur = ret;
  }
  return ret;
}

int max_of_four_int(int a, int b, int c, int d){
  int ret = a;
  if(ret < b)
    ret = b;
  if(ret < c)
    ret = c;
  if(ret < d)
    ret = d;
  return ret;
}


bool create(const char *name, off_t initial_size){
  address_check(name);
  bool success = filesys_create(name, initial_size);

  return success;
}

int open(const char *name){
  address_check(name);
  lock_acquire(&access_lock);
  struct file *fp = filesys_open(name);
  if(fp == NULL){
    lock_release(&access_lock);
    return -1;
  }
  
  if(strcmp(thread_current()->name, name) == 0)
    file_deny_write(fp);

  for(int i=3;i<128;i++){
    if(thread_current()->fd_table[i] == NULL){
      thread_current()->fd_table[i] = fp;
      lock_release(&access_lock);
      return i;
    }
  }
  lock_release(&access_lock);
  return -1;
}

bool remove(const char *name){
  address_check(name);

  bool success = filesys_remove(name);

  return success;
}

int filesize(int fd){
  struct file *fp = thread_current()->fd_table[fd];
  if(fp == NULL) exit(-1);
  return file_length(fp);
}

void seek(int fd, unsigned position){
  struct file *fp = thread_current()->fd_table[fd];
  if(fp == NULL) exit(-1);
  file_seek(fp, position);
}

unsigned tell(int fd){
  struct file *fp = thread_current()->fd_table[fd];
  if(fp == NULL) exit(-1);
  return file_tell(fp);
}

void close(int fd){
  struct file *fp = thread_current()->fd_table[fd];
  if(fp == NULL)
    exit(-1);
  file_close(fp);
  thread_current()->fd_table[fd] = NULL;
}
