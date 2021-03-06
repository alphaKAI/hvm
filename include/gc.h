#ifndef __GC_HEADER_INCLUDED__
#define __GC_HEADER_INCLUDED__

#include <stddef.h>

void gc_init(void);

void *gc_malloc(size_t size);

void *gc_realloc(void *ptr, size_t size);

void gc_free(void *ptr);

#endif
