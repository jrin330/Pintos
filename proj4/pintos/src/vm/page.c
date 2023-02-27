#include "vm/page.h"
#include "threads/pte.h"
#include "threads/palloc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vm/frame.h"
#include "threads/malloc.h"
#include "threads/thread.h"
#include "threads/flags.h"
#include "threads/interrupt.h"
#include "userprog/pagedir.h"
#include "threads/vaddr.h"
#include "vm/swap.h"
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "filesys/directory.h"
#include "devices/block.h"

struct pte* add_pte(void *vaddr, bool writable){
  struct pte *pte = malloc(sizeof *pte);
  if(pte == NULL) return NULL;

  pte->addr = pg_round_down(vaddr);
  pte->swap_ofs = (block_sector_t)-1;
  pte->writable = writable;
  pte->owner = thread_current();
  pte->frame = NULL;

  pte->file = NULL;
  pte->file_ofs = 0;
  pte->file_bytes = 0;
  if(hash_insert(thread_current()->my_pages, &pte->elem) != NULL){
    free(pte);
    pte = NULL;
  }

  return pte;
}

struct pte* get_pte(void *addr){
  struct pte pte;
  struct hash_elem *e;

  pte.addr = pg_round_down(addr);
  e = hash_find(thread_current()->my_pages, &pte.elem);
  
  if(e != NULL){
    return hash_entry(e, struct pte, elem);
  }
  return NULL;
}

void* get_frame_addr(void* addr){
  struct pte* f = get_pte(addr);
  return f->frame->addr;
} 

bool push_page(struct pte* pte){
  //if page is not mapped
  //allocate new frame
  if(pte->frame == NULL){
    pte->frame = allocate_frame(pte);
    if(pte->frame == NULL) {
	    return false;
    }
    pte->frame->pte = pte;
  }
  if(is_in_disk(pte)){
    write_from_disk(pte);
  }
  else if(pte->file != NULL){
    off_t read_byte = file_read_at(pte->file, pte->frame->addr,
		    pte->file_bytes, pte->file_ofs);
    off_t zero = PGSIZE - read_byte;
    memset(pte->frame->addr + read_byte, 0, zero);
  }
  else{
    memset(pte->frame->addr, 0, PGSIZE);
  }
  return true;
}

bool evict_page(struct pte *pte){
  pagedir_clear_page(pte->owner->pagedir, pte->addr);
  bool is_dirty = pagedir_is_dirty(pte->owner->pagedir, pte->addr);
  bool success = true;
  if(pte->file == NULL){
    success = write_to_disk(pte);
  }
  else{
    if(is_dirty){
      success = file_write_at(pte->file, pte->frame->addr, pte->file_bytes, pte->file_ofs);
    }
  }
  if(success){
    pte->frame = NULL;
  }
  return success;
}

void free_pte_all(void){
  if(thread_current()->my_pages != NULL)
    hash_destroy(thread_current()->my_pages, dealloc_pte);
}

void dealloc_pte(struct hash_elem* elem, void *aux UNUSED){
  struct pte* pte = hash_entry(elem, struct pte, elem);
  if(pte->frame != NULL)
    pte->frame->pte = NULL;

  free(pte);
}



uint32_t page_hash(const struct hash_elem *e, void *aux UNUSED){
  struct pte *p = hash_entry(e, struct pte, elem);
  return (uintptr_t)p->addr;
}

bool page_less(const struct hash_elem *a, const struct hash_elem *b, void *aux UNUSED){
  struct pte *ap = hash_entry(a, struct pte, elem);
  struct pte *bp = hash_entry(b, struct pte, elem);

  return ap->addr > bp->addr;
}

bool is_stack_growable(void *addr, void *esp){
  if(PHYS_BASE - pg_round_down(addr) >= MAX_STACK_SIZE){
    return false;
  }
  
  if(addr < esp - 32){
    return false;
  }
  
  return true;
}
