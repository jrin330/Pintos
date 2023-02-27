#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <stdbool.h>
#include <stdint.h>
#include "devices/block.h"
#include "filesys/off_t.h"
#include "filesys/file.h"
#include "threads/synch.h"
#include <hash.h>
#include <debug.h>

#define MAX_STACK_SIZE (1 << 23)

struct pte{
  struct hash_elem elem;
  void *addr;
  block_sector_t swap_ofs;

  struct thread *owner;
  struct frame *frame;
  bool writable;

  struct file* file;
  off_t file_ofs;
  off_t file_bytes;
};

struct pte* add_pte(void *vaddr, bool writable);
struct pte* get_pte(void *addr);
void* get_frame_addr(void *addr);
bool push_page(struct pte* pte);
bool evict_page(struct pte *pte);
bool is_stack_growable(void *addr, void* esp);

void free_pte_all(void);
void dealloc_pte(struct hash_elem* elem, void *aux UNUSED);

uint32_t page_hash(const struct hash_elem *e, void *aux UNUSED);
bool page_less(const struct hash_elem *a, const struct hash_elem *b, void *aux UNUSED);
#endif
