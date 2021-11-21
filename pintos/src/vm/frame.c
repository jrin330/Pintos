#include "vm/frame.h"
#include <stdio.h>
#include "vm/page.h"
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "threads/pte.h"
#include "threads/vaddr.h"
#include "vm/swap.h"
#include <debug.h>
#include <hash.h>

static struct hash all_frame;

void init_frame(void){
	printf("frame init enter\n");
  hash_init(&all_frame, frame_hash, frame_less, NULL);
  printf("all_frame complete\n");
  clock_init(&all_frame);
  printf("clock complete\n");
}


struct frame* allocate_frame(struct pte* pte){
  struct frame *f = malloc(sizeof *f);
  if(f == NULL) return NULL;

  f->addr = palloc_get_page(PAL_USER);
  f->pte = pte;

  if(f->addr != NULL){
    hash_insert(&all_frame, &f->elem);
    return f;
  }

  //if space is not available
  //evict frame by lru(second-chance)
  struct frame *evicted = get_frame_by_lru();
  if(!evict_page(evicted->pte)) return NULL;
  else{
    hash_delete(&all_frame, &evicted->elem);
    f->addr = palloc_get_page(PAL_USER);
    return f;
  }


  free(f);
  return NULL;
}

struct frame* rotate_frame(struct hash_iterator *i){
  if(hash_next(i) == NULL)
    hash_first(i, &all_frame);

  return hash_entry(hash_cur(i), struct frame, elem);
}



uint32_t frame_hash(const struct hash_elem *e, void *aux UNUSED){
  struct frame *f = hash_entry(e, struct frame, elem);
  return ((uint32_t)f->addr) >> PGBITS;
}

bool frame_less(const struct hash_elem *a, const struct hash_elem *b, void *aux UNUSED){
  struct frame *af = hash_entry(a, struct frame, elem);
  struct frame *bf = hash_entry(b, struct frame, elem);

  return af->addr < bf->addr;
}

