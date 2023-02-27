#include "vm/frame.h"
#include <stdio.h>
#include "vm/page.h"
#include "threads/thread.h"
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "threads/pte.h"
#include "threads/vaddr.h"
#include "vm/swap.h"
#include "userprog/pagedir.h"

static struct frame* all_frame;
static size_t frame_cnt;
static size_t clock;


void init_frame(size_t size){
  void *addr;
  frame_cnt = 0;
  all_frame = malloc(sizeof(struct frame)* size);
  while((addr = palloc_get_page(PAL_USER)) != NULL){
    all_frame[frame_cnt].addr = addr;
    all_frame[frame_cnt].pte = NULL;
    frame_cnt++;
  }
  clock = 0;
}

struct frame* allocate_frame(struct pte* pte){
  struct frame* f = NULL;
  for(int i=0;i<frame_cnt;i++){
    f = &all_frame[i];
    if(f->pte == NULL){
      f->pte = pte;
      return f;
    }
  }
  //no free frame
  //eviction
  while(1){
  struct frame* evicted = get_frame_by_lru();
    if(evict_page(evicted->pte)){
      evicted->pte = pte;
      return evicted;
    }
  }
  return NULL;
}

struct frame* get_frame_by_lru(void){
  while(1){
    if(pagedir_is_accessed(thread_current()->pagedir, all_frame[clock].pte)){
      pagedir_set_accessed(thread_current()->pagedir, all_frame[clock].pte, false);
    }
    else{
      return &all_frame[clock];
    }
    if(++clock >= frame_cnt) clock = 0;
  }
}
