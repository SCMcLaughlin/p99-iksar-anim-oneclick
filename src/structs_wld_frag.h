
#ifndef STRUCTS_WLD_FRAG_H
#define STRUCTS_WLD_FRAG_H

#include "define.h"

#ifdef PLATFORM_WINDOWS
#pragma warning(disable: 4200)
#endif

#pragma pack(1)

typedef struct WldColor {
    uint8_t b, g, r, a;
} WldColor;

typedef struct Vec3 {
    float   x, y, z;
} Vec3;

typedef struct Frag {
    uint32_t    length;
    uint32_t    type;
    int         nameRef;
} Frag;

#define frag_length(f) ((f)->length + (sizeof(uint32_t) * 2))

typedef struct FragSimpleRef {
    Frag        frag;
    int         ref;
    uint32_t    flag;
} FragSimpleRef;

typedef FragSimpleRef Frag05;
typedef FragSimpleRef Frag11;
typedef FragSimpleRef Frag2d;
typedef FragSimpleRef Frag2f;
typedef FragSimpleRef Frag33;

typedef struct Frag03 {
    Frag        frag;
    uint32_t    flag;
    uint16_t    stringLength;
    char        string[0];
} Frag03;

typedef struct Frag04 {
    Frag        frag;
    uint32_t    flag;
    int         count;
    int         ref;
} Frag04;

typedef struct Frag04Animated {
    Frag        frag;
    uint32_t    flag;
    int         count;
    uint32_t    milliseconds;
    int         refList[0];
} Frag04Animated;

typedef struct Frag10Bone {
    int         nameRef;
    uint32_t    flag;
    int         refA;
    int         refB;
    int         size;
} Frag10Bone;

typedef struct Frag10 {
    Frag        frag;
    uint32_t    flag;
    int         count;
    int         ref;
} Frag10;

typedef struct Frag12Entry {
    struct {
        int16_t w, x, y, z;
    } rot;
    
    struct {
        int16_t x, y, z, denom;
    } shift;
} Frag12Entry;

typedef struct Frag12 {
    Frag        frag;
    uint32_t    flag;
    uint32_t    count;
    Frag12Entry entries[0];
} Frag12;

typedef struct Frag13 {
    Frag        frag;
    int         ref;
    uint32_t    flag;
    uint32_t    framerate;
} Frag13;

typedef struct Frag14 {
    Frag        frag;
    uint32_t    flag;
    int         refA;
    int         skippableCount;
    int         meshRefCount;
    int         refB;
} Frag14;

typedef struct Frag15 {
    Frag        frag;
    int         objNameRef;
    uint32_t    flag;
    int         refB;
    Vec3        pos;
    Vec3        rot;
    Vec3        scale;
    int         refC;
    uint32_t    refCParam;
} Frag15;

typedef struct Frag30 {
    Frag        frag;
    uint32_t    flag;
    uint32_t    blendFlag;
    uint32_t    unknownA;       /* 0x00b2b2b2 (mobs, objects) or 0x004e4e4e (zones) */
    float       unknownB[2];    /* [0] = 0, 1, or 0.1; [1] = 0.75 or 0 */
    int         ref;
    int         unknownC[2];    /* Always seem to be 0 */
} Frag30;

typedef struct Frag31 {
    Frag        frag;
    uint32_t    flag;
    uint32_t    count;
    int         refList[0];
} Frag31;

typedef struct Frag32 {
    Frag        frag;
    uint32_t    flag;
    uint32_t    count;
    uint32_t    unknown[3];
    WldColor    colors[0];
} Frag32;

typedef struct Frag36 {
    Frag        frag;
    uint32_t    flag;
    int         materialListRef;
    int         animVertRef;
    int         unknownA[2];
    Vec3        pos;
    Vec3        rot;
    float       maxDist;
    Vec3        min;
    Vec3        max;
    uint16_t    vertexCount;
    uint16_t    uvCount;
    uint16_t    normalCount;
    uint16_t    colorCount;
    uint16_t    triangleCount;
    uint16_t    boneAssignmentCount;
    uint16_t    triangleTextureCount;
    uint16_t    vertexTextureCount;
    uint16_t    size;
    uint16_t    scale;
} Frag36;

#pragma pack()

#endif/*STRUCTS_WLD_FRAG_H*/
