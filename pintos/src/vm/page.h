#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <stdbool.h>
#include <stdint.h>
#include "devices/block.h"
#include "filesys/off_t.h"
#include "threads/synch.h"
#include <hash.h>

struct pte{
  struct hash_elem elem;
  void *addr;
  block_sector_t swap_ofs;

  struct thread *owner;
  struct frame *frame;
  bool writable;
};

struct pte* add_pte(void *vaddr, bool writable);
struct pte* get_pte(void *addr);
bool push_page(void *fault_addr);
bool evict_page(struct pte *pte);

uint32_t page_hash(const struct hash_elem *e, void *aux);
bool page_less(const struct hash_elem *a, const struct hash_elem *b, void *aux);
#endif
