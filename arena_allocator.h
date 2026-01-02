#ifndef INCLUDE_ARENA_ALLOCATOR_H
#define INCLUDE_ARENA_ALLOCATOR_H

#include "ret_code.h"
#include <stdlib.h>
#include <stdint.h>

typedef struct {
    uint8_t* data;
    size_t   capacity;
    size_t   offset;
} ArenaAllocator;

Errors ArenaAllocator_init(ArenaAllocator* dest, size_t capacity);
Errors ArenaAllocator_deinit(ArenaAllocator* dest);

void* ArenaAllocator_allocate(ArenaAllocator* dest, size_t size, size_t align);
Errors ArenaAllocator_reset(ArenaAllocator* dest);

#endif  // INCLUDE_ARENA_ALLOCATOR_H