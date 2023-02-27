#ifndef VM_FRAME_H
#define VM_FRAME_H

#include <stdbool.h>
#include <hash.h>
#include <debug.h>

//physical page
struct frame{
  void* addr;
  struct pte *pte;
  struct hash_elem elem;
};


void init_frame(void);

struct frame* allocate_frame(struct pte* pte);
struct frame* rotate_frame(struct hash_iterator *i);


unsigned frame_hash(const struct hash_elem *e, void *aux UNUSED);
bool frame_less(const struct hash_elem *a, const struct hash_elem *b, void *aux UNUSED);

#endif
