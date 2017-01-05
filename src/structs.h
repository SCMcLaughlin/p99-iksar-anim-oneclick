
#ifndef STRUCTS_H
#define STRUCTS_H

#include "define.h"
#include "bit.h"
#include "structs_container.h"
#include "structs_wld_frag.h"

typedef struct Pfs {
    Array   entries;
    HashTbl byName;
    Buffer* raw;
    Buffer* path;
} Pfs;

typedef struct Wld {
    Array   fragsByIndex;
    HashTbl fragsByNameRef;
    char*   strings;
    int     stringsLength;
    Buffer* data;
} Wld;

typedef struct WldHeader {
    uint32_t signature;
    uint32_t version;
    uint32_t fragCount;
    uint32_t unknownA[2];
    uint32_t stringsLength;
    uint32_t unknownB;
} WldHeader;

typedef struct StringBlock {
    HashTbl indexByString;
    char*   strings;
    int     nextIndex;
} StringBlock;

typedef struct RefMap {
    HashTbl     old2new;
    StringBlock strBlock;
    Wld*        srcWld;
} RefMap;

typedef struct VirtualWld {
    RefMap      refMap;
    byte*       fragsRaw;
    uint32_t    length;
    uint32_t    fragCount;
} VirtualWld;

#endif/*STRUCTS_H*/
