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

static void syscall_handler (struct intr_frame *);

struct lock access_lock;
void
syscall_init (void) 
{
	lock_init(&access_lock);
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  if(!is_user_vaddr(f->esp)) exit(-1);
  uint32_t callnum = *(uint32_t*)f->esp;

  //hex_dump((uintptr_t)f->esp, f->esp, PHYS_BASE - f->esp, true);
  switch(callnum){
    case SYS_HALT:
      halt();break;
    case SYS_EXIT:
      if(!is_user_vaddr(f->esp + 4)) exit(-1);
      exit(*(uint32_t*)(f->esp + 4)); break;
          case SYS_EXEC:
        if(!is_user_vaddr(f->esp + 4)) exit(-1);
        if(!is_user_vaddr(*(char**)(f->esp + 4))) exit(-1);
        f->eax = exec(*(char**)(f->esp + 4)); break;
    case SYS_WAIT:
        if(!is_user_vaddr(f->esp + 4)) exit(-1);
        f->eax = wait(*(tid_t*)(f->esp + 4)); break;
    case SYS_CREATE:
        if(!is_user_vaddr(f->esp + 4)) exit(-1);
        if(!is_user_vaddr(*(void **)(f->esp + 4))) exit(-1);
        f->eax = create(*(char **)(f->esp + 4), *(uint32_t*)(f->esp + 8));
        break;
    case SYS_REMOVE:
        if(!is_user_vaddr(f->esp + 4)) exit(-1);
        if(!is_user_vaddr(*(void **)(f->esp + 4))) exit(-1);
        f->eax = remove(*(char **)(f->esp + 4));
        break;
    case SYS_OPEN:
        if(!is_user_vaddr(f->esp + 4)) exit(-1);
        f->eax = open(*(char **)(f->esp + 4));
        break;
    case SYS_CLOSE:
        if(!is_user_vaddr(f->esp + 4)) exit(-1);
        close(*(int *)(f->esp + 4));
        break;
    case SYS_FILESIZE:
        if(!is_user_vaddr(f->esp + 4)) exit(-1);
        f->eax = filesize(*(int *)(f->esp + 4));
        break;
    case SYS_SEEK:
        if(!is_user_vaddr(f->esp + 8)) exit(-1);
        seek(*(int*)(f->esp + 4), *(unsigned *)(f->esp + 8));
        break;
    case SYS_TELL:
        if(!is_user_vaddr(f->esp + 4)) exit(-1);
        f->eax = tell(*(int*)(f->esp + 4));
        break;
    case SYS_READ:
        if(!is_user_vaddr(f->esp + 12)) exit(-1);
        if(!is_user_vaddr(*(void**)(f->esp + 8))) exit(-1);
        f->eax = read(*(uint32_t*)(f->esp + 4), *(void**)(f->esp + 8), (unsigned)*(uint32_t*)(f->esp + 12)); break;
    case SYS_WRITE:
        if(!is_user_vaddr(f->esp + 12)) exit(-1);
        if(!is_user_vaddr(*(void**)(f->esp + 8))) exit(-1);
        f->eax = write(*(uint32_t*)(f->esp + 4), *(void**)(f->esp + 8), (unsigned)*(uint32_t*)(f->esp + 12)); break;
    case SYS_FIBO:
        if(!is_user_vaddr(f->esp + 4)) exit(-1);
        f->eax = fibonacci(*(uint32_t*)(f->esp + 4));
        break;
	    case SYS_MAXF:
        if(!is_user_vaddr(f->esp + 16)) exit(-1);
         f->eax = max_of_four_int(*(uint32_t*)(f->esp + 4), *(uint32_t*)(f->esp + 8), *(uint32_t*)(f->esp + 12), *(uint32_t*)(f->esp + 16));break;
  }


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
  char file_name[128];
  char *delimp;
  if((delimp = strchr(cmd_line, ' ')) == NULL)
    strlcpy(file_name, cmd_line, strlen(cmd_line) + 1);
  else
    strlcpy(file_name ,cmd_line, delimp - cmd_line + 1);

  if(filesys_open(file_name) == NULL)
    return -1;

  return process_execute(cmd_line);
}

int wait(tid_t pid){
  return process_wait(pid);
}

int read(int fd, void *buffer, unsigned size)
{
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
  lock_acquire(&access_lock);
  int ret = -1;
  if(fd == 1){
    putbuf(buffer, size);
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
  if(name == NULL) exit(-1);
  return filesys_create(name, initial_size);
}
int open(const char *name){
  if(name == NULL) exit(-1);
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
  if(name == NULL) exit(-1);
  return filesys_remove(name);
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

