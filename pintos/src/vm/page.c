#include "vm/page.h"
#include "threads/pte.h"
#include "threads/palloc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vm/frame.h"
#include "threads/malloc.h"
#include "threads/thread.h"
#include "userprog/pagedir.h"
#include "threads/vaddr.h"
#include "vm/swap.h"


struct pte* add_pte(void *vaddr, bool writable){
  struct pte *pte = malloc(sizeof *pte);
  if(pte == NULL) return NULL;

  pte->addr = pg_round_down(vaddr);
  pte->swap_ofs = (block_sector_t) -1;
  pte->writable = writable;
  pte->owner = thread_current();

  if(hash_insert(thread_current()->my_pages, &pte->elem) != NULL){
    free(pte);
    pte = NULL;
  }

  return pte;
}

struct pte* get_pte(void *addr){
  if(is_kernel_vaddr(addr)) return NULL;

  struct pte pte;
  struct hash_elem *e;

  pte.addr = pg_round_down(addr);
  e = hash_find(thread_current()->my_pages, &pte.elem);
  if(e != NULL)
    return hash_entry(e, struct pte, elem);

  return NULL;
}

bool push_page(void *fault_addr){
  struct pte *pte;
  bool success;

  pte = get_pte(fault_addr);
  if(pte == NULL) return false;

  //if page is not mapped
  //allocate new frame
  if(pte->frame == NULL){
    pte->frame = alloc_frame(pte);
    if(pte->frame == NULL) return false;

    if(pte->swap_ofs != (block_sector_t)-1){
    // if frame's contents are in swap disk

      write_from_disk(pte);
    }
    else{
      memset(pte->frame->addr, 0, PGSIZE);
    }
  }

  //mapping frame with page table
  success = pagedir_set_page(thread_current()->pagedir, pte->addr, 
		  pte->frame->addr, pte->writable);
  return success;
}

bool evict_page(struct pte *pte){
  pagedir_clear_page(pte->owner->pagedir, pte->addr);

  bool is_dirty = pagedir_is_dirty(pte->owner->pagedir, pte->addr);

  bool success = write_to_disk(pte);

  if(success) pte->frame = NULL;

  return success;
}

uint32_t page_hash(const struct hash_elem *e, void *aux){
  struct pte *p = hash_entry(e, struct pte, elem);
  return ((uint32_t)p->addr) >> PGBITS;
}

bool page_less(const struct hash_elem *a, const struct hash_elem *b, void *aux){
  struct pte *ap = hash_entry(a, struct pte, elem);
  struct pte *bp = hash_entry(b, struct pte, elem);

  return ap->addr < bp->addr;
}
