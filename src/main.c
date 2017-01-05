
#include "define.h"
#include "pfs.h"
#include "wld.h"
#include "virtual_wld.h"

#define TARGET_PFS "global4_chr.s3d"
#define BACKUP_PFS "global4_chr.zae"
#define TARGET_WLD "global4_chr.wld"

static void output(FILE* fp, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vfprintf(fp, fmt, args);
    va_end(args);
    fflush(fp);
}

static const char* errmsg(int rc)
{
    switch (rc)
    {
    case ERR_Invalid:
    case ERR_OutOfBounds:
        return "File data was malformed";
    case ERR_CouldNotOpen:
        return "Could not open file";
    case ERR_OutOfMemory:
        return "Out of memory";
    default:
        return "Unknown error";
    }
}

static int save_raw(Pfs* pfs, const char* path)
{
    Buffer* raw = pfs->raw;
    FILE* fp = fopen(path, "wb+");
    int rc = ERR_None;
    
    if (!fp) return ERR_CouldNotOpen;
    
    if (fwrite(buf_data(raw), sizeof(byte), buf_length(raw), fp) != buf_length(raw))
        rc = ERR_FileOperation;
    
    fclose(fp);
    return rc;
}

static int main_open(Pfs* pfs)
{
    int rc = pfs_open(pfs, BACKUP_PFS);
    
    if (rc == ERR_None)
    {
        output(stdout, "Backup file '" BACKUP_PFS "' already exists; using it as a base\n");
        return ERR_None;
    }
    
    pfs_close(pfs); /* Just in case the backup file was partially opened somehow */
    rc = pfs_open(pfs, TARGET_PFS);
    
    if (rc)
    {
        if (rc == ERR_CouldNotOpen)
            output(stderr, "Could not open '" TARGET_PFS "': make sure you're running this from your EQ folder!\n");
        else
            output(stderr, "Could not open '" TARGET_PFS "': %s\n", errmsg(rc));
        
        return rc;
    }
    
    output(stdout, "Found '" TARGET_PFS "'\nCreating backup file '" BACKUP_PFS "'...\n");
    rc = save_raw(pfs, BACKUP_PFS);
    
    if (rc)
        output(stderr, "Error creating backup: %s\n", errmsg(rc));
    
    return rc;
}

static int modify_wld(VirtualWld* vwld, Wld* wld)
{
    Array* frags = &wld->fragsByIndex;
    Array delayed;
    uint32_t i = 0;
    Frag** ptr;
    int rc;
    
    array_init(&delayed, Frag*);
    
    while ( (ptr = array_get(frags, ++i, Frag*)) )
    {
        Frag* frag = *ptr;
        const char* name = wld_frag_name(wld, frag);
        uint32_t len = 0;
        
        if (name)
        {
            len = strlen(name);
            
            if (len >= 6 && (frag->type == 0x13 || frag->type == 0x12))
            {
                if (name[0] == 'C' && name[1] == '0' && (name[2] == '3' || name[2] == '9') && name[3] == 'I' && name[4] == 'K' && name[5] == 'M')
                {
                    if (frag->type == 0x13)
                    {
                        if (!array_push_back(&delayed, (void*)&frag))
                        {
                            rc = ERR_OutOfMemory;
                            goto abort;
                        }
                    }
                    
                    continue;
                }
            }
        }
        
        rc = vwld_add_old_frag(vwld, i, frag, name, len);
        if (rc) goto abort;
    }
    
    i = 0;
    while ( (ptr = array_get(&delayed, i++, Frag*)) )
    {
        Frag13* f13 = (Frag13*)*ptr;
        Frag12* f12 = (Frag12*)wld_frag_by_ref(wld, f13->ref);
        const char* name = wld_frag_name(wld, &f12->frag);
        
        rc = vwld_add_new_frag(vwld, f12, name);
        if (rc) return rc;
        
        f13->ref = vwld_last_added_ref(vwld);
        rc = vwld_add_new_frag(vwld, f13, wld_frag_name(wld, &f13->frag));
        if (rc) return rc;
    }
    
    array_deinit(&delayed, NULL);
    return ERR_None;
    
abort:
    array_deinit(&delayed, NULL);
    output(stderr, "Error while modifying '" TARGET_WLD "': %s\n", errmsg(rc));
    return rc;
}

static int process_wld(Pfs* pfs, Buffer* data)
{
    Wld wld;
    VirtualWld vwld;
    Buffer* saved;
    int rc = wld_open(&wld, data);
    
    if (rc)
    {
        output(stderr, "Error loading internal datafile '" TARGET_WLD "': %s\n", errmsg(rc));
        wld_close(&wld);
        return rc;
    }
    
    output(stdout, "Opened internal datafile '" TARGET_WLD "'\n");
    
    rc = vwld_init(&vwld, &wld);
    
    if (rc)
    {
        output(stderr, "Preparations to modify '" TARGET_WLD "' failed: %s\n", errmsg(rc));
        goto abort;
    }
    
    rc = modify_wld(&vwld, &wld);
    if (rc) goto abort;
    
    saved = vwld_save(&vwld);
    
    if (!saved)
    {
        output(stderr, "Could not write modified '" TARGET_WLD "': Out of memory\n");
        rc = ERR_OutOfMemory;
        goto abort;
    }
    
    rc = pfs_put(pfs, TARGET_WLD, sizeof(TARGET_WLD) - 1, buf_data(saved), buf_length(saved));
    
    if (rc)
    {
        output(stderr, "Could not re-insert modified '" TARGET_WLD "': %s\n", errmsg(rc));
        goto no_insert;
    }
    
    output(stdout, "Modified '" TARGET_WLD "' successfully\n");
    
    rc = pfs_save_as(pfs, TARGET_PFS);
    
    if (rc)
    {
        output(stderr, "Error while saving changes to '" TARGET_PFS "': %s\n", errmsg(rc));
        goto no_insert;
    }
    
    output(stdout, "Saved changes to '" TARGET_PFS "'\n");
    rc = ERR_None;
    
no_insert:
    buf_destroy(saved);
abort:
    vwld_deinit(&vwld);
    wld_close(&wld);
    return rc;
}

static int main_process(Pfs* pfs)
{
    uint32_t i = 0;
    
    for (;;)
    {
        Buffer* name = pfs_get_name(pfs, i++);
        Buffer* data;
        
        if (!name) break;
        
        if (strcmp(buf_str(name), TARGET_WLD) != 0)
            continue;
        
        data = pfs_get(pfs, buf_str(name), buf_length(name));
        
        if (!data)
        {
            output(stderr, "Decompression of internal datafile '" TARGET_WLD "' failed\n");
            return ERR_Invalid;
        }
        
        return process_wld(pfs, data);
    }
    
    output(stderr, "Could not find internal datafile '" TARGET_WLD "'\n");
    return ERR_Invalid;
}

int main(void)
{
    int rc;
    Pfs pfs;
    
    rc = (main_open(&pfs) || main_process(&pfs));
    
    if (rc)
        output(stderr, "Aborting\n");
    else
        output(stdout, "Success\n");
    
    pfs_close(&pfs);
    
#ifdef PLATFORM_WINDOWS
    output(stdout, "\nPress a key to exit...\n");
    getchar();
#endif
    
    return rc ? EXIT_FAILURE : EXIT_SUCCESS;
}
