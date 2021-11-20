#include "vm/page.h"
#include "threads/pte.h"
#include "threads/palloc.h"
#include <stdio.h>
#include <stdlib.h>
#include "vm/frame.h"
#include "threads/malloc.h"
#include "threads/thread.h"
#include "userprog/pagedir.h"
#include "threads/vaddr.h"


struct pte* add_pte(void *vaddr, bool writable){
  struct pte *p = malloc(sizeof *p);
  if(p == NULL) return NULL;

  p->addr = pg_round_down(vaddr);
  p->swap_ofs = (block_sector_t) -1;
  p->writable = writable;
  p->owner = thread_current();

  if(hash_insert(thread_current()->my_page, &elem) != NULL){
    free(p);
    p = NULL;
  }

  return p;
}

struct pte* get_pte(void *addr){
  if(is_kernel_address(addr)) return NULL;

  struct pte p;
  struct hash_elem *e;

  p.addr = pg_round_down(addr);
  e = hash_find(thread_current()->my_page, &p.hash_elem);
  if(e != NULL)
    return hash_entry(e, struct pte, hash_elem);

  return NULL;
}

bool push_page(void *fault_addr){
  struct pte *p;
  bool success;

  p = get_pte(fault_addr);
  if(p == NULL) return false;

  //if page is not mapped
  //allocate new frame
  if(p->frame == NULL){
    p->frame = alloc_frame(p);
    if(f == NULL) return false;

    if(p->swap_ofs != -1){
    // if frame's contents are in swap disk

      write_from_disk(p);
    }
    else{
      memset(p->frame->addr, 0, PGSIZE);
    }
  }

  //mapping frame with page table
  success = pagedir_set_page(thread_current()->pagedir, p->addr, 
		  p->frame->addr, p->writable);
  return success;
}

bool evict_page(struct pte *pte){
  pagedir_clear_page(pte->owner->pagedir, pte->addr);

  bool is_dirty = pagedir_is_dirty(pte->owner->pagedir, pte->addr);

  bool success = write_to_disk(pte);

  if(success) pte->frame = NULL;

  return success;
}

uint32_t get_pte_info(uint32_t *pd, struct pte* pte){
  uint32_t *pde = pd + pd_no(pte->addr);
  uint32_t *pt = pde_get_pt(*pde);
  return pt[pt_no(pte->addr)];
}

uint32_t page_hash(const struct hash_elem *e, void *aux){
  struct page *p = hash_entry(e, struct page, hash_elem);
  return ((uint32_t*)p->addr) >> PGBITS;
}

bool page_less(const struct hash_elem *a, const struct hash_elem *b, void *aux){
  struct page *ap = hash_entry(a, struct page, hash_elem);
  struct page *bp = hash_entry(b, struct page, hash_elem);

  return ap->addr < bp->addr;
}
