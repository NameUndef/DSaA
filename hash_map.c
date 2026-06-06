#include "hash_map.h"
#include "ret_code.h"
#include "alignment.h"

#include <stdalign.h>
#include <stdlib.h>
#include <string.h>

#define DEF_INIT_MAP_SIZE 1024
#define BYTE_SIZE 8

int HashMap_init(HashMap *obj, SlabAllocator *unitialized_slab_allocator, PageAllocator *page_allocator, size_t init_map_size)
{
    if (obj == NULL) {
        return BAD_ARGUMENT_NULL_POINTER;
    }

    if (unitialized_slab_allocator == NULL) {
        return BAD_ARGUMENT_NULL_POINTER;
    }

    if (page_allocator == NULL) {
        return BAD_ARGUMENT_NULL_POINTER;
    }

    if (!init_map_size) {
        init_map_size = DEF_INIT_MAP_SIZE;
    }

    if (!is_power_of_2(init_map_size)) {
        return BAD_ARGUMENT_NOT_POWER_OF_2;
    }

    BucketNode **map = (BucketNode**) calloc(init_map_size, sizeof(BucketNode*));
    if (!map) {
        return ALLOC_ERROR;
    }

    int ret_code = SlabAllocator_init(unitialized_slab_allocator, page_allocator, sizeof(BucketNode), alignof(BucketNode), 0);
    if (ret_code) {
        free(map);
        return ret_code;
    }

    obj->bucket_node_allocator = unitialized_slab_allocator;
    obj->map = map;
    obj->map_size = init_map_size;
    obj->used = 0;
 
    return OK;
}

int HashMap_deinit(HashMap *obj)
{
    if (!obj) {
        return BAD_ARGUMENT_NULL_POINTER;
    }

    free(obj->map);
    SlabAllocator_deinit(obj->bucket_node_allocator);

    return OK;
}

static size_t HM_get_hash(const char *key)
{
    size_t hash = 5381;
    for (size_t i = 0; key[i]; i++) {
        hash = ((hash << 5) + hash) + key[i];
    }

    return hash;
}

static size_t HM_get_index(size_t hash, size_t map_size)
{
    return hash & (map_size - 1);
}

static BucketNode* HM_try_find_in_bucket(BucketNode *bucket, const char *key)
{
    BucketNode *node = bucket;

    while (node) {
        
        if (strcmp(key, node->key) == 0) {
            break;
        }

        node = node->next;
    }

    return node;
}

static BucketNode* HM_AddNode(HashMap *obj, BucketNode** map_place, BucketNode *bucket)
{
    BucketNode *new_node = (BucketNode*) SlabAllocator_allocate(obj->bucket_node_allocator);
    if (!new_node) {
        return NULL;
    }

    new_node->prev = NULL;
    new_node->next = bucket;

    if (bucket) {
        bucket->prev = new_node;
    }

    *map_place = new_node;
    return new_node;
}

static int HM_rehash(HashMap *obj, size_t new_size)
{
    BucketNode **map = (BucketNode**) calloc(new_size, sizeof(BucketNode*));
    if (!map) {
        return ALLOC_ERROR;
    }

    for (size_t i = 0; i < obj->map_size; i++) {

        BucketNode *node = obj->map[i];

        while (node) {

            BucketNode *next_node = node->next;

            size_t hash = HM_get_hash(node->key);
            size_t idx = HM_get_index(hash, new_size);

            BucketNode **target_map_cell = map + idx;
            BucketNode *target_bucket = *target_map_cell;

            node->next = target_bucket;
            node->prev = NULL;

            if (target_bucket) {
                target_bucket->prev = node;
            }

            *target_map_cell = node;
            node = next_node;
        }
    }

    free(obj->map);
    obj->map = map;
    obj->map_size = new_size;

    return OK;
}

int HashMap_set(HashMap *obj, const char *key, int data)
{
    if (!obj) {
        return BAD_ARGUMENT_NULL_POINTER;
    }

    if (!key) {
        return BAD_ARGUMENT_NULL_POINTER;
    }

    size_t hash = HM_get_hash(key);
    size_t idx = HM_get_index(hash, obj->map_size);

    BucketNode **map_place = obj->map + idx;
    BucketNode *bucket = *map_place;

    BucketNode *node = HM_try_find_in_bucket(bucket, key);

    if (node) {
        node->data = data;
    } else {
        node = HM_AddNode(obj, map_place, bucket);
        strncpy(node->key, key, KEY_SIZE);
        node->data = data;
        obj->used++;

        double coef = (double) obj->used / obj->map_size;

        if (coef >= 0.75) {

            if (obj->map_size & 1ULL << (sizeof(size_t) * BYTE_SIZE - 1)) {
                return OK;
            }

            int ret_code = HM_rehash(obj, obj->map_size << 1);
            if (ret_code) {
                return ret_code;
            }
        }
    }

    return OK;
}

int HashMap_get(HashMap *obj, const char *key, int *data)
{
    if (!obj) {
        return BAD_ARGUMENT_NULL_POINTER;
    }

    if (!key) {
        return BAD_ARGUMENT_NULL_POINTER;
    }

    if (!data) {
        return BAD_ARGUMENT_NULL_POINTER;
    }

    size_t hash = HM_get_hash(key);
    size_t idx = HM_get_index(hash, obj->map_size);

    BucketNode *node = obj->map[idx];
    node = HM_try_find_in_bucket(node, key);

    if (node) {
        *data = node->data;
    } else {
        return NOT_FOUND;
    }

    return OK;
}

int HashMap_is_have(HashMap *obj, const char *key, bool *result)
{
    if (!obj) {
        return BAD_ARGUMENT_NULL_POINTER;
    }

    if (!key) {
        return BAD_ARGUMENT_NULL_POINTER;
    }

    if (!result) {
        return BAD_ARGUMENT_NULL_POINTER;
    }

    size_t hash = HM_get_hash(key);
    size_t idx = HM_get_index(hash, obj->map_size);

    BucketNode *node = obj->map[idx];
    node = HM_try_find_in_bucket(node, key);

    *result = !!node;

    return OK;
}

int HashMap_remove(HashMap *obj, const char *key)
{
    if (!obj) {
        return BAD_ARGUMENT_NULL_POINTER;
    }

    if (!key) {
        return BAD_ARGUMENT_NULL_POINTER;
    }

    size_t hash = HM_get_hash(key);
    size_t idx = HM_get_index(hash, obj->map_size);

    BucketNode **map_place = obj->map + idx;
    BucketNode *bucket = *map_place;

    BucketNode *node = HM_try_find_in_bucket(bucket, key);

    if (!node) {
        return NOT_FOUND;
    }

    if (node->prev) {
        node->prev->next = node->next;
    } else {
        *map_place = node->next;
    }

    if (node->next) {
        node->next->prev = node->prev;
    }

    SlabAllocator_free(node);

    return OK;
}
