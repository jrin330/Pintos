#ifndef VM_FRAME_H
#define VM_FRAME_H

#include <stdbool.h>
#include <hash.h>
#include <debug.h>
#include "threads/synch.h"
//physical page
struct frame{
  void* addr;
  struct pte *pte;
};


void init_frame(size_t size);

struct frame* allocate_frame(struct pte* pte);
struct frame* get_frame_by_lru(void);

#endif
