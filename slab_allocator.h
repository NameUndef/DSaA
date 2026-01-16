#ifndef INCLUDE_SLAB_ALLOCATOR_H
#define INCLUDE_SLAB_ALLOCATOR_H

#include "ret_code.h"
#include "page_allocator.h"
#include <stdlib.h>
#include <stdint.h>

struct _SlabCache;

typedef struct _Slab {
    struct _SlabCache* cache;
    struct _Slab*      prev;
    struct _Slab*      next;
    uint8_t*           free_list; // указывает на свободные объекты в слабе, в каждом свободном объекте указатель на следующий свободный
    size_t             used;
    uint8_t            slots_data[]; // выровнен
} Slab;

typedef struct _SlabCache {
    PageAllocator* page_allocator;
    size_t         object_count;
    size_t         object_size;
    size_t         object_alignment;    // проверить, нужно ли поле или нет
    size_t         first_object_padding;
    size_t         aligned_object_size;
    Slab*          empty;
    Slab*          partial;
    Slab*          full;
} SlabCache;

typedef SlabCache SlabAllocator;

Errors SlabAllocator_init(
    SlabAllocator* dest, 
    PageAllocator* page_allocator,
    size_t object_size, 
    size_t object_alignment,
    size_t init_cache_capacity);
Errors SlabAllocator_deinit(SlabAllocator* dest);
void* SlabAllocator_allocate(SlabAllocator* dest);
Errors SlabAllocator_free(void* data);

#endif  // INCLUDE_SLAB_ALLOCATOR_H