
#ifndef UTIL_BUFFER_H
#define UTIL_BUFFER_H

#include "define.h"
#include "util_alloc.h"
#include "structs_container.h"

Buffer* buf_create(const void* data, uint32_t len);
#define buf_destroy(buf) free(buf)
Buffer* buf_from_file(const char* path);
Buffer* buf_from_file_ptr(FILE* fp);

uint32_t buf_length(Buffer* buf);
const byte* buf_data(Buffer* buf);
byte* buf_writable(Buffer* buf);
const char* buf_str(Buffer* buf);
char* buf_str_writable(Buffer* buf);

#endif/*UTIL_BUFFER_H*/
