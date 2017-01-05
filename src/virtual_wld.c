
#include "virtual_wld.h"

/* StringBlock */

int strblk_init(StringBlock* strblk)
{
    static const char firstString[] = "By Zaela (lol)";
    uint32_t size = bit_next_pow2_u32(sizeof(firstString));
    
    tbl_init(&strblk->indexByString, int);
    strblk->strings = alloc_array_type(size, char);
    
    if (!strblk->strings)
        return ERR_OutOfMemory;
    
    memcpy(strblk->strings, firstString, sizeof(firstString));
    strblk->nextIndex = sizeof(firstString);
    return ERR_None;
}

void strblk_deinit(StringBlock* strblk)
{
    tbl_deinit(&strblk->indexByString, NULL);
    
    if (strblk->strings)
    {
        free(strblk->strings);
        strblk->strings = NULL;
    }
}

static int strblk_check_realloc(StringBlock* strblk, int len)
{
    int fence = bit_next_pow2_u32(strblk->nextIndex);
    char* ptr;
    
    if (len < fence)
        return ERR_None;
    
    while (fence < len) fence *= 2;
    
    ptr = realloc_array_type(strblk->strings, fence, char);
    if (!ptr) return ERR_OutOfMemory;
    
    strblk->strings = ptr;
    return ERR_None;
}

int strblk_add(StringBlock* strblk, const char* str, uint32_t len, int* out)
{
    int* index = tbl_get_str(&strblk->indexByString, str, len, int);
    int rc;
    int newNext;
    
    if (index)
    {
        if (out)
            *out = *index;
        return ERR_None;
    }
    
    rc = tbl_set_str(&strblk->indexByString, str, len, &strblk->nextIndex);
    
    if (rc == ERR_Again)
    {
        rc = tbl_remove_str(&strblk->indexByString, str, len);
        if (rc) return rc;
        rc = tbl_set_str(&strblk->indexByString, str, len, &strblk->nextIndex);
    }
    
    if (rc) return rc;
    
    len++;
    newNext = strblk->nextIndex + len;
    rc = strblk_check_realloc(strblk, newNext);
    if (rc) return rc;
    
    if (out)
        *out = strblk->nextIndex;
    
    memcpy(strblk->strings + strblk->nextIndex, str, len);
    strblk->nextIndex = newNext;
    return ERR_None;
}

int strblk_get_index(StringBlock* strblk, const char* str, uint32_t len, int* out)
{
    int* index = tbl_get_str(&strblk->indexByString, str, len, int);
    if (!index) return ERR_OutOfBounds;
    
    *out = *index;
    return ERR_None;
}

/* RefMap */

int rmap_init(RefMap* rmap, Wld* wld)
{
    tbl_init(&rmap->old2new, int);
    rmap->srcWld = wld;
    return strblk_init(&rmap->strBlock);
}

void rmap_deinit(RefMap* rmap)
{
    tbl_deinit(&rmap->old2new, NULL);
    strblk_deinit(&rmap->strBlock);
}

int rmap_set(RefMap* rmap, int oldRef, int newRef)
{
    return tbl_update_int(&rmap->old2new, oldRef, &newRef);
}

int rmap_get(RefMap* rmap, int oldRef, int* out)
{
    int* ptr = tbl_get_int(&rmap->old2new, oldRef, int);
    
    if (ptr)
    {
        *out = *ptr;
        return ERR_None;
    }
    
    if (oldRef == 0)
    {
        *out = 0;
        return ERR_None;
    }
    
    if (oldRef < 0)
    {
        const char* name = wld_name_by_ref(rmap->srcWld, oldRef);
        
        if (name)
        {
            int index;
            int rc = strblk_add(&rmap->strBlock, name, strlen(name), &index);
            if (rc) return rc;
            
            index = -index;
            *out = index;
            return tbl_update_int(&rmap->old2new, oldRef, &index);
        }
    }
    
    return ERR_Invalid;
}

/* VirtualWld */

int vwld_init(VirtualWld* vwld, Wld* wld)
{
    memset(vwld, 0, sizeof(VirtualWld));
    return rmap_init(&vwld->refMap, wld);
}

void vwld_deinit(VirtualWld* vwld)
{
    rmap_deinit(&vwld->refMap);
    
    if (vwld->fragsRaw)
    {
        free(vwld->fragsRaw);
        vwld->fragsRaw = NULL;
    }
}

static int vwld_fix_simp(RefMap* rmap, Frag* frag)
{
    FragSimpleRef* f = (FragSimpleRef*)frag;
    return rmap_get(rmap, f->ref, &f->ref);
}

static int vwld_fix_f04(RefMap* rmap, Frag* frag)
{
    Frag04* f04 = (Frag04*)frag;
    
    if (f04->count > 1)
    {
        Frag04Animated* f04a = (Frag04Animated*)f04;
        int i;
        
        for (i = 0; i < f04a->count; i++)
        {
            int rc = rmap_get(rmap, f04a->refList[i], &f04a->refList[i]);
            if (rc) return rc;
        }
        
        return ERR_None;
    }
    
    return rmap_get(rmap, f04->ref, &f04->ref);
}

static int vwld_fix_f10(RefMap* rmap, Frag* frag)
{
    Frag10* f10 = (Frag10*)frag;
    byte* ptr = ((byte*)f10) + sizeof(Frag10);
    int* iptr;
    int rc, i, n;
    
    rc = rmap_get(rmap, f10->ref, &f10->ref);
    if (rc) return rc;
    
    if (f10->flag & 1)
        ptr += 12;
    
    if (f10->flag & 2)
        ptr += 4;
    
    n = f10->count;
    for (i = 0; i < n; i++)
    {
        Frag10Bone* bone = (Frag10Bone*)ptr;
        ptr += bone->size * 4 + sizeof(Frag10Bone);
        
        rc = rmap_get(rmap, bone->nameRef, &bone->nameRef);
        if (rc) return rc;
        rc = rmap_get(rmap, bone->refA, &bone->refA);
        if (rc) return rc;
        rc = rmap_get(rmap, bone->refB, &bone->refB);
        if (rc) return rc;
    }
    
    iptr = (int*)ptr;
    n = *iptr++;
    for (i = 0; i < n; i++)
    {
        rc = rmap_get(rmap, *iptr, iptr);
        iptr++;
        if (rc) return rc;
    }
    
    return ERR_None;
}

static int vwld_fix_f13(RefMap* rmap, Frag* frag)
{
    Frag13* f13 = (Frag13*)frag;
    return rmap_get(rmap, f13->ref, &f13->ref);
}

static int vwld_fix_f14(RefMap* rmap, Frag* frag)
{
    Frag14* f14 = (Frag14*)frag;
    int* ptr = (int*)(((byte*)f14) + sizeof(Frag14));
    int rc, i, n;
    
    rc = rmap_get(rmap, f14->refA, &f14->refA);
    if (rc) return rc;
    rc = rmap_get(rmap, f14->refB, &f14->refB);
    if (rc) return rc;
    
    if (f14->meshRefCount == 0)
        return ERR_None;
    
    if (f14->flag & 1)
        ptr++;
    
    if (f14->flag & 2)
        ptr++;
    
    n = f14->skippableCount;
    for (i = 0; i < n; i++)
    {
        ptr += (*ptr) * 2 + 1;
    }
    
    n = f14->meshRefCount;
    for (i = 0; i < n; i++)
    {
        rc = rmap_get(rmap, *ptr, ptr);
        ptr++;
        if (rc) return rc;
    }
    
    return ERR_None;
}

static int vwld_fix_f30(RefMap* rmap, Frag* frag)
{
    Frag30* f30 = (Frag30*)frag;
    return rmap_get(rmap, f30->ref, &f30->ref);
}

static int vwld_fix_f31(RefMap* rmap, Frag* frag)
{
    Frag31* f31 = (Frag31*)frag;
    uint32_t i;
    
    for (i = 0; i < f31->count; i++)
    {
        int rc = rmap_get(rmap, f31->refList[i], &f31->refList[i]);
        if (rc) return rc;
    }
    
    return ERR_None;
}

static int vwld_fix_f36(RefMap* rmap, Frag* frag)
{
    Frag36* f36 = (Frag36*)frag;
    int rc = rmap_get(rmap, f36->materialListRef, &f36->materialListRef);
    return (rc) ? rc : rmap_get(rmap, f36->animVertRef, &f36->animVertRef);
}

static int vwld_fix_refs(RefMap* rmap, Frag* frag)
{
    switch (frag->type)
    {
    case 0x05:
    case 0x11:
    case 0x2d:
    case 0x33:
        return vwld_fix_simp(rmap, frag);
    
    case 0x04:
        return vwld_fix_f04(rmap, frag);
    
    case 0x10:
        return vwld_fix_f10(rmap, frag);
    
    case 0x13:
        return vwld_fix_f13(rmap, frag);
    
    case 0x14:
        return vwld_fix_f14(rmap, frag);
    
    case 0x30:
        return vwld_fix_f30(rmap, frag);
    
    case 0x31:
        return vwld_fix_f31(rmap, frag);
    
    case 0x36:
        return vwld_fix_f36(rmap, frag);
    
    default:
        return ERR_None;
    }
}

static int vwld_check_realloc(VirtualWld* vwld, uint32_t newTotal)
{
    uint32_t fence = bit_next_pow2_u32(vwld->length);
    byte* ptr;
    
    if (newTotal < fence)
        return ERR_None;
    
    if (fence == 0)
        fence = 1;
    
    while (fence < newTotal) fence *= 2;
    
    ptr = realloc_bytes(vwld->fragsRaw, fence);
    if (!ptr) return ERR_OutOfMemory;
    
    vwld->fragsRaw = ptr;
    return ERR_None;
}

int vwld_add_old_frag(VirtualWld* vwld, int oldIndex, void* frag, const char* name, uint32_t namelen)
{
    Frag* f = (Frag*)frag;
    uint32_t len;
    int rc;
    int noFix = false;
    
    if (name)
    {
        int nameref;
        rc = strblk_add(&vwld->refMap.strBlock, name, namelen, &nameref);
        if (rc) return rc;
        
        nameref = -nameref;
        
        if (f->nameRef != (int)0xffffffff)
        {
            rc = rmap_set(&vwld->refMap, f->nameRef, nameref);
            if (rc) return rc;
        }
        else
        {
            noFix = true;
        }
        
        f->nameRef = nameref;
    }
    
    if (!noFix)
    {
        rc = vwld_fix_refs(&vwld->refMap, f);
        if (rc) return rc;
    }
    
    len = vwld->length + frag_length(f);
    rc = vwld_check_realloc(vwld, len);
    if (rc) return rc;
    
    memcpy(vwld->fragsRaw + vwld->length, f, frag_length(f));
    vwld->length = len;
    vwld->fragCount++;
    return rmap_set(&vwld->refMap, oldIndex, vwld->fragCount);
}

int vwld_add_new_frag(VirtualWld* vwld, void* frag, const char* name)
{
    Frag* f = (Frag*)frag;
    f->nameRef = (int)0xffffffff;
    return vwld_add_old_frag(vwld, (int)vwld->fragCount, frag, name, name ? strlen(name) : 0);
}

Buffer* vwld_save(VirtualWld* vwld)
{
    StringBlock* strblk = &vwld->refMap.strBlock;
    WldHeader header;
    Buffer* buf = buf_create(NULL, sizeof(header) + strblk_length(strblk) + vwld->length);
    byte* ptr;
    
    if (!buf) return NULL;
    
    ptr = buf_writable(buf);
    
    memcpy(&header, buf_data(vwld->refMap.srcWld->data), sizeof(header));
    
    header.fragCount = vwld->fragCount;
    header.stringsLength = strblk_length(strblk);
    memcpy(ptr, &header, sizeof(header));
    ptr += sizeof(header);
    
    memcpy(ptr, strblk_strings(strblk), strblk_length(strblk));
    wld_process_string(ptr, strblk_length(strblk));
    ptr += strblk_length(strblk);
    
    memcpy(ptr, vwld->fragsRaw, vwld->length);
    return buf;
}
