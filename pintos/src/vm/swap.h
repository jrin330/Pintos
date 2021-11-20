#ifndef VM_SWAP_H
#define VM_SWAP_H

#include <stdbool.h>

struct page;
struct frame;

extern hash_iterator *clock;

void disk_init(void);
void write_from_disk(struct pte *pte);
bool write_to_disk(struct pte *pte);

#endif
