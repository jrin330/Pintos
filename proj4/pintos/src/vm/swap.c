#include "vm/swap.h"
#include "devices/block.h"
#include <hash.h>
#include <bitmap.h>
#include "vm/frame.h"
#include "vm/page.h"
#include "threads/vaddr.h"
#include "threads/thread.h"
#include "userprog/pagedir.h"
#include "userprog/syscall.h"
#include <stdio.h>

static struct block *swap_disk;
static struct bitmap *swap_bitmap;
#define PGSEC (PGSIZE / BLOCK_SECTOR_SIZE)

void disk_init(void){
  swap_disk = block_get_role(BLOCK_SWAP);
  if(swap_disk == NULL) 
    swap_bitmap = bitmap_create(0);
  else
    swap_bitmap = bitmap_create(block_size(swap_disk)/PGSEC);

}

bool is_in_disk(struct pte* pte){
  if(pte->swap_ofs != (block_sector_t)-1)
    return true;
  else
    return false;
}
void write_from_disk(struct pte* pte){
  for(int i=0;i<PGSEC; i++){
    block_read(swap_disk, pte->swap_ofs + i, 
		    pte->frame->addr + i*BLOCK_SECTOR_SIZE);
  }
  bitmap_reset(swap_bitmap, pte->swap_ofs / PGSEC);
  pte->swap_ofs = (block_sector_t)-1;
}

bool write_to_disk(struct pte *pte){
  size_t block_idx = bitmap_scan_and_flip(swap_bitmap, 0, 1, false);
  if(block_idx == BITMAP_ERROR){
	  return false;
  }
  pte->swap_ofs = block_idx * PGSEC;
  for(int i=0; i<PGSEC; i++){
    block_write(swap_disk, pte->swap_ofs + i, pte->frame->addr + i*BLOCK_SECTOR_SIZE);
  }
  pte->file = NULL;
  return true;
}

