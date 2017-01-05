
#ifndef PFS_H
#define PFS_H

#include "define.h"
#include "structs.h"
#include "util_container.h"
#include "util_alloc.h"
#include "crc.h"
#include <zlib.h>

int pfs_open(Pfs* pfs, const char* path);
void pfs_close(Pfs* pfs);
int pfs_save(Pfs* pfs);
int pfs_save_as(Pfs* pfs, const char* path);

Buffer* pfs_get(Pfs* pfs, const char* name, uint32_t len);
int pfs_put(Pfs* pfs, const char* name, uint32_t namelen, const void* data, uint32_t datalen);

Buffer* pfs_get_name(Pfs* pfs, uint32_t index);

#endif/*PFS_H*/
