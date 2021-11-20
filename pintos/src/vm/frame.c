#include "vm/frame.h"
#include <stdio.h>
#include "vm/page.h"
#include "thread.malloc.h"
#include "threads/palloc.h"
#include "threads/pte.h"
#include "threads/vaddr.h"
#include "vm/swap.h"

struct hash *all_frame;


struct frame* alloc_frame(struct pte* pte){
  struct frame *f = malloc(sizeof *f);
  if(f == NULL) return NULL;

  f->addr = palloc_get_page(PAL_USER);
  f->pte = pte;

  if(f->addr != NULL){
    hash_insert(all_frame, &f->elem);
    return f;
  }

  //if space is not available
  //evict frame by lru(second-chance)
  struct frame *evicted = get_frame_by_lru(all_frame);
  if(!evict_page(evicted->pte)) return NULL;
  else{
    f->addr = palloc_get_page(PAL_USER);
    return f;
  }


  free(f);
  return NULL;
}
