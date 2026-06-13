#include "buddy_allocator.h"
#include "page_manage.h"
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

    shift_t cur_size_shift = (shift_t) get_0_bits_count_before_1(cur_size);
    shift_t min_size_shift = (shift_t) get_0_bits_count_before_1(min_size);

    while (cur_size_shift >= min_size_shift) {
        size_t end_idx = idx + count;
        while (idx != end_idx) {
            tree[idx].max_size_shift = cur_size_shift;
            idx++;
        }
        cur_size_shift--;
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

    if (minimum_alloc_size == 1 && leafs_count > (1ULL << ((sizeof(size_t) * BYTE_SIZE) - 2))) {
        return BAD_ARGUMENT_OUT_OF_BOUNDS;
    }

    size_t nodes_count = (leafs_count << 1) - 1;

    if (((size_t) -1) / nodes_count < sizeof(BuddyNode)) {
        return BAD_ARGUMENT_OUT_OF_BOUNDS;
    }

    shift_t data_size_shift = (shift_t) get_0_bits_count_before_1(size);

    BuddyNode* tree = (BuddyNode*) allocate_page_sz(nodes_count * sizeof(BuddyNode));
    if (!tree) {
        return ALLOC_ERROR;
    }

    void *data = allocate_page_sz(size);
    if (!data) {
        free_page_sz(tree, nodes_count * sizeof(BuddyNode));
        return ALLOC_ERROR;
    }

    BA_init_tree(tree, size, minimum_alloc_size);

    obj->data = data;
    obj->data_size = size;
    obj->minimum_alloc_size = minimum_alloc_size;
    obj->nodes_tree = tree;
    obj->nodes_count = nodes_count;
    obj->data_size_shift = data_size_shift;

    return OK;
}

int BuddyAllocator_deinit(BuddyAllocator *obj)
{
    free_page_sz(obj->data, obj->data_size);
    free_page_sz(obj->nodes_tree, obj->nodes_count * sizeof(BuddyNode));

    obj->data = NULL;
    obj->data_size = 0;
    obj->minimum_alloc_size = 0;
    obj->nodes_tree = NULL;
    obj->nodes_count = 0;
    obj->data_size_shift = 0;

    return OK;
}

void* BuddyAllocator_allocate(BuddyAllocator *obj, size_t size)
{
    if (size > (1ULL << (sizeof(size_t) * BYTE_SIZE -1))) {
        return NULL;
    }

    if (size > obj->data_size) {
        return NULL;
    }

    if (size < obj->minimum_alloc_size) {
        size = obj->minimum_alloc_size;
    } 

    size_t aligned_size = ceil_to_power_of_2(size);
    shift_t target_shift = (shift_t) get_0_bits_count_before_1(aligned_size);

    if (obj->nodes_tree[0].max_size_shift < target_shift) {
        return NULL;
    }

    size_t cur_node = 0;
    size_t offset = 0;
    size_t actual_pos = obj->data_size;

    do {
        actual_pos = actual_pos >> 1;

        size_t left = 2 * cur_node + 1;
        size_t right = left + 1;

        shift_t left_child_shift = obj->nodes_tree[left].max_size_shift;
        shift_t right_child_shift = obj->nodes_tree[right].max_size_shift;

        if (left_child_shift <= right_child_shift && left_child_shift >= target_shift) {
            cur_node = left;
        } else if (right_child_shift >= target_shift) {
            cur_node = right;
            offset |= actual_pos;
        }

        if (actual_pos == aligned_size) {
            break;
        }

    } while (true);

    size_t upd_node = cur_node;
    obj->nodes_tree[upd_node].max_size_shift = 0;
    if (upd_node) {

        do {

            upd_node = (upd_node - 1) / 2;

            shift_t left_shift = obj->nodes_tree[upd_node * 2 + 1].max_size_shift;
            shift_t right_shift = obj->nodes_tree[upd_node * 2 + 2].max_size_shift;
            size_t parent = obj->nodes_tree[upd_node].max_size_shift;

            shift_t max = left_shift >= right_shift? left_shift : right_shift;
            if (max == parent) {
                break;
            }

            obj->nodes_tree[upd_node].max_size_shift = max;
            
            if (!upd_node) {
                break;
            }

        } while (true);
    }

    void *result_ptr = obj->data + offset;
    return result_ptr;
}

int BuddyAllocator_free(BuddyAllocator *obj, void *data)
{
    char *ptr = (char*) data;
    ptrdiff_t ptr_diff = ptr - obj->data;
    if (ptr_diff < 0) {
        return BAD_ARGUMENT_OUT_OF_BOUNDS;
    }

    size_t offset = (size_t) ptr_diff;
    if (offset >= obj->data_size) {
        return BAD_ARGUMENT_OUT_OF_BOUNDS;
    }

    size_t target_idx = 0;
    shift_t offset_shift = 0;

    do {

        shift_t parent_shift = obj->nodes_tree[target_idx].max_size_shift;
        size_t right_child_idx = 2 * target_idx + 2;

        if (right_child_idx > obj->nodes_count) {
            break;
        }

        shift_t right_child_shift = obj->nodes_tree[right_child_idx].max_size_shift;

        if (!parent_shift && right_child_shift) {
            break;
        }

        bool to_left = !(bool)((offset >> offset_shift) & 1);
        target_idx = right_child_shift - (shift_t) (to_left);
        offset_shift++;

    } while (true);

    shift_t actual_shift = obj->data_size_shift - offset_shift;

    obj->nodes_tree[target_idx].max_size_shift = actual_shift;

    do {
        if (target_idx < 2) {
            break;
        }

        target_idx = (target_idx - 1) / 2;

        size_t left_child_idx = 2 * target_idx + 1;
        size_t right_child_idx = left_child_idx + 1;
        shift_t left_child_shift = obj->nodes_tree[left_child_idx].max_size_shift;
        shift_t right_child_shift = obj->nodes_tree[right_child_idx].max_size_shift;
        shift_t target_idx_shift = obj->nodes_tree[target_idx].max_size_shift;
        shift_t next_actual_shift = actual_shift + 1;

        if (left_child_shift == right_child_shift) {
            if (left_child_shift == actual_shift) {
                obj->nodes_tree[target_idx].max_size_shift = next_actual_shift;
            } else {
                break;
            }
        } else {
            shift_t max = left_child_shift > right_child_shift? left_child_shift : right_child_shift;
            if (max > target_idx_shift) {
                obj->nodes_tree[target_idx].max_size_shift = max;
            } else {
                break;
            }
        }
        
        actual_shift = next_actual_shift;

    } while (true);

    return OK;
}
