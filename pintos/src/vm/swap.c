#include "vm/swap.h"
#include "devices/block.h"
#include <hash.h>
#include <bitmap.h>
#include "vm/frame.h"
#include "vm/page.h"
#include "threads/vaddr.h"
#include "threads/thread.h"

struct block *swap_disk;
struct bitmap *swap_bitmap;
#define PGSEC (PGSIZE / BLOCK_SECTOR_SIZE)

struct hash_iterator *clock;

void disk_init(void){
  swap_disk = block_get_role(BLOCK_SWAP);
  swap_bitmap = bitmap_create(block_size(swap_disk)/PGSEC);
  if(swap_bitmap == NULL) 
    thread_exit();
}

void write_from_disk(struct pte* pte){
  for(int i=0;i<PGSEC; i++){
    block_read(swap_disk, pte->swap_ofs + i, 
		    pte->frame->addr + i*BLOCK_SECTOR_SIZE);
  }
  pte->swap_ofs = -1;
}

bool write_to_disk(struct pte *pte){
  size_t block_idx = bitmap_scan_and_flip(swap_bitmap, 0, 1, false);
  if(block_idx == BITMAP_ERROR) return false;
  
  pte->swap_ofs = block_idx;
  for(int i=0; i<PGSEC; i++){
    block_write(swap_disk, pte->swap_ofs + i, pte->frame->addr + i*BLOCK_SECTOR_SIZE);
  }

  return true;
}

struct frame* get_frame_by_lru(struct hash *h){



}
