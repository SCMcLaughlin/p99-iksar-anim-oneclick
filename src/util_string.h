
#ifndef UTIL_STRING_H
#define UTIL_STRING_H

#include "define.h"
#include "bit.h"
#include "util_alloc.h"
#include "structs_container.h"

void str_init(String* str);
void str_deinit(String* str);

uint32_t str_length(String* str);
const char* str_data(String* str);

int str_set(String* str, const char* cstr, uint32_t len);
#define str_set_literal(str, lit) str_set((str), (lit), sizeof(lit) - 1)
int str_set_from_file(String* str, const char* path);
int str_set_from_file_ptr(String* str, FILE* fp);

int str_append(String* str, const char* input, uint32_t len);
int str_append_char(String* string, char c);
#define str_append_literal(str, lit) str_append((str), (lit), sizeof(lit) - 1)
int str_append_file(String* str, const char* path);
int str_append_file_ptr(String* str, FILE* fp);

int str_reserve(String* str, uint32_t bytes);
void str_clear(String* str);
void str_subtract_chars(String* str, uint32_t numChars);

#endif/*UTIL_STRING_H*/
