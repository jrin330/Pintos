#ifndef VM_SWAP_H
#define VM_SWAP_H

#include <stdbool.h>
#include <hash.h>

struct pte;
struct frame;

extern struct hash_iterator *clock;

void disk_init(void);
void write_from_disk(struct pte* pte);
bool write_to_disk(struct pte* pte);

struct frame* get_frame_by_lru(struct hash *);

#endif
