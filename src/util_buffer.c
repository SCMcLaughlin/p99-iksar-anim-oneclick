
#include "util_buffer.h"

Buffer* buf_create(const void* data, uint32_t len)
{
    uint32_t dlen   = len + sizeof(uint32_t) + 1; /* Include null terminator for string data */
    byte* ptr       = alloc_bytes(dlen);
    uint32_t* plen  = (uint32_t*)ptr;
    
    if (!ptr) return NULL;
    
    *plen = len;
    
    if (data && len)
        memcpy(ptr + sizeof(uint32_t), data, len);
    
    ptr[dlen - 1] = 0; /* Explicit null terminator */
    
    return (Buffer*)ptr;
}

Buffer* buf_from_file(const char* path)
{
    FILE* fp = fopen(path, "rb");
    Buffer* buf;
    
    if (!fp) return NULL;
    
    buf = buf_from_file_ptr(fp);
    
    fclose(fp);
    return buf;
}

Buffer* buf_from_file_ptr(FILE* fp)
{
    long len;
    uint32_t dlen;
    byte* ptr;
    uint32_t* plen;
    
    if (fseek(fp, 0, SEEK_END))
        return NULL;
    
    len = ftell(fp);
    
    if (len <= 0 || fseek(fp, 0, SEEK_SET))
        return NULL;
    
    dlen    = (uint32_t)len + sizeof(uint32_t) + 1;
    ptr     = alloc_bytes(dlen);
    plen    = (uint32_t*)ptr;
    
    if (!ptr)
        return NULL;
    
    *plen = (uint32_t)len;
    
    if (fread(ptr + sizeof(uint32_t), sizeof(byte), (size_t)len, fp) != (size_t)len)
    {
        free(ptr);
        return NULL;
    }
    
    return (Buffer*)ptr;
}

uint32_t buf_length(Buffer* buf)
{
    uint32_t* len = (uint32_t*)buf;
    return *len;
}

const byte* buf_data(Buffer* buf)
{
    const byte* data = (const byte*)buf;
    return data + sizeof(uint32_t);
}

byte* buf_writable(Buffer* buf)
{
    return (byte*)buf_data(buf);
}

const char* buf_str(Buffer* buf)
{
    return (const char*)buf_data(buf);
}

char* buf_str_writable(Buffer* buf)
{
    return (char*)buf_data(buf);
}
