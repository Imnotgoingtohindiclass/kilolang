#pragma once
#include <stddef.h>

void  gc_init(void);
void *gc_alloc(size_t);
void  gc_collect(void);