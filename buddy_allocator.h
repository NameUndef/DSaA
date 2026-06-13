#ifndef INCLUDE_BUDDY_ALLOCATOR
#define INCLUDE_BUDDY_ALLOCATOR

#include <stddef.h>
#include <stdbool.h>

typedef unsigned char shift_t;

typedef struct {
    shift_t max_size_shift;
} BuddyNode;

typedef struct {
    char* data;
    size_t data_size;
    BuddyNode* nodes_tree;
    size_t nodes_count;
    size_t minimum_alloc_size;
    shift_t data_size_shift;
} BuddyAllocator;

int BuddyAllocator_init(BuddyAllocator *obj, size_t size, size_t minimum_alloc_size);
int BuddyAllocator_deinit(BuddyAllocator *obj);
void* BuddyAllocator_allocate(BuddyAllocator *obj, size_t size);
int BuddyAllocator_free(BuddyAllocator *obj, void *data);

#endif  // INCLUDE_BUDDY_ALLOCATOR