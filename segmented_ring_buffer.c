#include "segmented_ring_buffer.h"
#include "container_of.h"
#include <stdlib.h>

static void SRB_pre_init(SegmentedRingBuffer *dest, size_t segment_capacity) 
{
    dest->first_segment = NULL;
    dest->cur_segment = NULL;
    dest->capacity = 0;
    dest->segment_capacity = segment_capacity;
}

static void SRB_post_deinit(SegmentedRingBuffer *dest)
{
    dest->first_segment = NULL;
    dest->cur_segment = NULL;
    dest->capacity = 0;
    dest->segment_capacity = 0;
}

static Errors SRB_add_last_segment(SegmentedRingBuffer *dest, ListNode *prev_node)
{
    Segment* new_segment = (Segment*) malloc(sizeof(Segment));
    if (!new_segment) {
        return ALLOC_ERROR;
    }

    Errors ret = RingBuffer_init(&new_segment->buffer, dest->segment_capacity);
    if (ret != OK) {
        free(new_segment);
        return ret;
    }

    new_segment->node.next_node = NULL;
    new_segment->node.prev_node = prev_node;

    if (prev_node) {
        prev_node->next_node = &new_segment->node;
    } else {
        dest->first_segment = new_segment;
    }

    return OK;
}

static void SRB_free_segments(SegmentedRingBuffer *dest)
{
    ListNode *cur_node = &dest->first_segment->node;
    while (cur_node) {
        ListNode* next_node = cur_node->next_node;
        Segment* segment = container_of(cur_node, Segment, node);

        RingBuffer_deinit(&segment->buffer);
        free(segment);
        cur_node = next_node;
    }
}

Errors SegmentedRingBuffer_init(SegmentedRingBuffer* dest, size_t segment_capacity, size_t opt_init_capacity)
{
    if (!dest) {
        return BAD_ARGUMENT_NULL_POINTER;
    }

    if (!segment_capacity) {
        return BAD_ARGUMENT_ZERO;
    }

    if (!segment_capacity) {
        return BAD_ARGUMENT_ZERO;
    }

    opt_init_capacity = opt_init_capacity ? opt_init_capacity - 1: 0;

    SRB_pre_init(dest, segment_capacity);

    Errors ret_code = SRB_add_last_segment(dest, NULL);
    if (ret_code != OK) {
        return ret_code;
    }

    ListNode* cur_node = &dest->first_segment->node;

    for (size_t i = 0; i < opt_init_capacity; i++) {

        Errors ret_code = SRB_add_last_segment(dest, cur_node);
        if (ret_code != OK) {
            SRB_free_segments(dest);
            return ret_code;
        }

        cur_node = cur_node->next_node;
    }

    dest->cur_segment = dest->first_segment;
    
    return OK;
}

Errors SegmentedRingBuffer_deinit(SegmentedRingBuffer* dest)
{
    if (!dest) {
        return BAD_ARGUMENT_NULL_POINTER;
    }

    SRB_free_segments(dest);
    SRB_post_deinit(dest);

    return OK;
}

Errors SegmentedRingBuffer_push(SegmentedRingBuffer* dest, uint8_t value)
{
    if (!dest) {
        return BAD_ARGUMENT_NULL_POINTER;
    }

    Errors ret_code = RingBuffer_push(&dest->cur_segment->buffer, value);
    if (ret_code == FULL_BUFFER) {

        if (!dest->cur_segment->node.next_node) {
            ret_code = SRB_add_last_segment(dest, &dest->cur_segment->node);
            if (ret_code != OK) {
                return ret_code;
            }
        }

        dest->cur_segment = container_of(dest->cur_segment->node.next_node, Segment, node);

        RingBuffer_push(&dest->cur_segment->buffer, value); // ignore return code, because it always OK
    }

    return OK;
}

Errors SegmentedRingBuffer_pop(SegmentedRingBuffer* dest, uint8_t* value)
{
    if (!dest) {
        return BAD_ARGUMENT_NULL_POINTER;    
    }

    Errors ret_code = RingBuffer_pop(&dest->cur_segment->buffer, value);
    if (ret_code == EMPTY_BUFFER) {

        if (!dest->cur_segment->node.prev_node) {
            return EMPTY_BUFFER;
        }

        dest->cur_segment = container_of(dest->cur_segment->node.prev_node, Segment, node);

        RingBuffer_pop(&dest->cur_segment->buffer, value); // ignore return code, because it always OK
    }

    return OK;
}

bool SegmentedRingBuffer_is_empty(SegmentedRingBuffer* dest)
{
    return dest && dest->cur_segment == dest->first_segment && RingBuffer_is_empty(&dest->cur_segment->buffer);
}
