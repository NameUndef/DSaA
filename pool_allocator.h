#ifndef INCLUDE_POOL_ALLOCATOR_H
#define INCLUDE_POOL_ALLOCATOR_H

#include "ret_code.h"
#include <stdlib.h>
#include <stdint.h>

typedef struct {
    size_t   pool_size;
    size_t   capacity_count;
    size_t   object_size;
    size_t   object_alignment_padding;
    size_t   object_aligned_size;
    size_t   used;
    uint8_t* free_list;
    uint8_t* mem_start;
} PoolAllocator;

Errors PoolAllocator_init(PoolAllocator* dest, size_t capacity_count, size_t object_size, size_t object_alignment);
Errors PoolAllocator_deinit(PoolAllocator* dest);
void*  PoolAllocator_allocate(PoolAllocator* dest);

// адрес должен в диапазоне пула и быть выровнен по объекту, иначе ошибка
// адрес должен быть на занятый участок, а не на свободны, иначе UB
Errors PoolAllocator_free(PoolAllocator* dest, void* address);

inline size_t PoolAllocator_get_used(PoolAllocator* dest)
{
    return dest->used;
}

inline size_t PoolAllocator_get_free(PoolAllocator* dest)
{
    return dest->capacity_count - dest->used;
}

#endif  // INCLUDE_POOL_ALLOCATOR_H