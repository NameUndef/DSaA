#include "buddy_allocator.h"
#include "ret_code.h"
#include "alignment.h"
#include "utils.h"

#define DEF_SIZE           4096
#define DEF_MIN_ALLOC_SIZE 64
#define BYTE_SIZE          8

static void BA_init_tree(BuddyNode *tree, size_t cur_size, size_t min_size)
{
    size_t count = 1;
    size_t idx = 0;

    size_t cur_size_shift = get_0_bits_count_before_1(cur_size);
    size_t min_size_shift = get_0_bits_count_before_1(min_size);

    while (cur_size_shift >= min_size_shift) {
        size_t end_idx = idx + count;
        while (idx != end_idx) {
            tree[idx].max_size_shift = cur_size;
            idx++;
        }
        cur_size--;
        count = count << 1;
    }
}

int BuddyAllocator_init(BuddyAllocator *obj, size_t size, size_t minimum_alloc_size)
{
    if (!obj) {
        return BAD_ARGUMENT_NULL_POINTER;
    }

    if (!size) {
        size = DEF_SIZE;
    }

    if (!minimum_alloc_size) {
        minimum_alloc_size = DEF_MIN_ALLOC_SIZE;
    }

    if (!is_power_of_2(size)) {
        return BAD_ARGUMENT_NOT_POWER_OF_2;
    }

    if (!is_power_of_2(minimum_alloc_size)) {
        return BAD_ARGUMENT_NOT_POWER_OF_2;
    }

    if (minimum_alloc_size > size) {
        return BAD_ARGUMENT_OUT_OF_BOUNDS;
    }

    size_t leafs_count = size >> get_0_bits_count_before_1(minimum_alloc_size);

    if (minimum_alloc_size == 1 && leafs_count > (1 << ((sizeof(size_t) * BYTE_SIZE) - 2))) {
        return BAD_ARGUMENT_OUT_OF_BOUNDS;
    }

    size_t nodes_count = (leafs_count << 1) - 1;

    if (((size_t) -1) / nodes_count < sizeof(BuddyNode)) {
        return BAD_ARGUMENT_OUT_OF_BOUNDS;
    }

    BuddyNode* tree = (BuddyNode*) malloc(nodes_count * sizeof(BuddyNode));
    if (!tree) {
        return ALLOC_ERROR;
    }

    void *data = (void*) malloc(size);
    if (!data) {
        free(tree);
        return ALLOC_ERROR;
    }

    BA_init_tree(tree, size, minimum_alloc_size);

    obj->data = data;
    obj->data_size = size;
    obj->minimum_alloc_size = minimum_alloc_size;
    obj->nodes_tree = tree;
    obj->nodes_count = nodes_count;

    return OK;
}

int BuddyAllocator_deinit(BuddyAllocator *obj)
{
    free(obj->data);
    free(obj->nodes_tree);

    obj->data = NULL;
    obj->data_size = 0;
    obj->minimum_alloc_size = 0;
    obj->nodes_tree = NULL;
    obj->nodes_count = NULL;

    return OK;
}

void* BuddyAllocator_allocate(BuddyAllocator *obj, size_t size)
{
    if (size > (1ULL << (sizeof(size_t) * BYTE_SIZE -1))) {
        return BAD_ARGUMENT_OUT_OF_BOUNDS;
    }

    if (size > obj->data_size) {
        return BAD_ARGUMENT_OUT_OF_BOUNDS;
    }

    if (size < obj->minimum_alloc_size) {
        size = obj->minimum_alloc_size;
    } 

    size_t aligned_size = ceil_to_power_of_2(size);

    if (aligned_size == obj->minimum_alloc_size) {

    } else {
        
    }

    return OK;
}

int BuddyAllocator_free(BuddyAllocator *obj, void *data)
{
    return OK;
}

