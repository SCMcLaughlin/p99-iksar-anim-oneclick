
#ifndef VIRTUAL_WLD_H
#define VIRTUAL_WLD_H

#include "define.h"
#include "structs.h"
#include "util_alloc.h"
#include "util_container.h"
#include "wld.h"

/* StringBlock */
int strblk_init(StringBlock* strblk);
void strblk_deinit(StringBlock* strblk);

int strblk_add(StringBlock* strblk, const char* str, uint32_t len, int* out);
int strblk_get_index(StringBlock* strblk, const char* str, uint32_t len, int* out);

#define strblk_strings(blk) ((blk)->strings)
#define strblk_length(blk) ((blk)->nextIndex)

/* RefMap */
int rmap_init(RefMap* rmap, Wld* wld);
void rmap_deinit(RefMap* rmap);

int rmap_set(RefMap* rmap, int oldRef, int newRef);
int rmap_get(RefMap* rmap, int oldRef, int* out);

/* VirtualWld */
int vwld_init(VirtualWld* vwld, Wld* wld);
void vwld_deinit(VirtualWld* vwld);

#define vwld_last_added_ref(vwld) ((vwld)->fragCount)

int vwld_add_old_frag(VirtualWld* vwld, int oldIndex, void* frag, const char* name, uint32_t namelen);
int vwld_add_new_frag(VirtualWld* vwld, void* frag, const char* name);
Buffer* vwld_save(VirtualWld* vwld);

#endif/*VIRTUAL_WLD_H*/
