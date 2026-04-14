#pragma once
#include <stddef.h>
#include <stdint.h>

void init_buddy_allocator(uint64_t, uint64_t);
void buddy_free(void*);
void* buddy_realloc(void*, size_t size);
void* buddy_alloc(size_t size);
