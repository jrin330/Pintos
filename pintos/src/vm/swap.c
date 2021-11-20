#include "vm/swap.h"
#include "devices/block.h"
#include <hash.h>
#include "vm/frame.h"
#include "vm/page.h"
#include "threads/vaddr.h"

struct block *swap_disk;

#define PGSEC (PGSIZE / BLOCK_SECTOR_SIZE)

void disk_init(void){
  swap_disk = block_get_role(BLOCK_SWAP);
}

void write_from_disk(struct pte* pte){
  for(int i=0;i<PGSEC; i++){
    block_read(swap_disk, pte->swap_ofs + i, 
		    pte->frame->addr + i*BLOCK_SECTOR_SIZE);
  }
  pte->swap_ofs = -1;
}

bool write_to_disk(struct pte *pte){



}
