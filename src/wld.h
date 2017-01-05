
#ifndef WLD_H
#define WLD_H

#include "define.h"
#include "structs.h"
#include "structs_wld_frag.h"
#include "util_container.h"

int wld_open(Wld* wld, Buffer* file);
void wld_close(Wld* wld);
void wld_process_string(void* str, uint32_t len);

const char* wld_frag_name(Wld* wld, Frag* frag);
const char* wld_name_by_ref(Wld* wld, int nameRef);
Frag* wld_frag_by_ref(Wld* wld, int ref);

#endif/*WLD_H*/
