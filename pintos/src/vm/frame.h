#ifndef VM_FRAME_H
#define VM_FRAME_H

#include <stdbool.h>

//physical page
struct frame{
  void* addr;
  struct pte *pte;
  struct hash_elem elem;
};

extern struct hash *all_frame;
struct frame* alloc_frame(struct pte* pte);
#endif
