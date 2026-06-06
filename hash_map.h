#ifndef INCLUDE_HASH_MAP
#define INCLUDE_HASH_MAP

#include "slab_allocator.h"
#include "page_allocator.h"
#include <stddef.h>
#include <stdbool.h>

#define KEY_SIZE 50

typedef struct _BucketNode {
    char key[KEY_SIZE];
    struct _BucketNode *next;
    struct _BucketNode *prev;
    int data;
} BucketNode;

typedef struct {
    SlabAllocator *bucket_node_allocator;
    BucketNode **map;
    size_t map_size;
    size_t used;
} HashMap;

int HashMap_init(HashMap *obj, SlabAllocator *unitialized_slab_allocator, PageAllocator *page_allocator, size_t init_map_size);
int HashMap_deinit(HashMap *obj);
int HashMap_set(HashMap *obj, const char *key, int data);
int HashMap_get(HashMap *obj, const char *key, int *data);
int HashMap_is_have(HashMap *obj, const char *key, bool *result);
int HashMap_remove(HashMap *obj, const char *key);

#endif  // INCLUDE_HASH_MAP