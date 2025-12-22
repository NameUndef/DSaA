#ifndef INCLUDE_SEGMENTED_RING_BUFFER_H
#define INCLUDE_SEGMENTED_RING_BUFFER_H

#include "ring_buffer.h"
#include "ret_code.h"

typedef struct _ListNode {
    struct _ListNode* next_node;
    struct _ListNode* prev_node;
} ListNode;

typedef struct {
    RingBuffer  buffer;
    ListNode    node;
} Segment;

typedef struct {
    Segment*    first_segment;
    Segment*    cur_segment;
    size_t      capacity;
    size_t      segment_capacity;
} SegmentedRingBuffer;

Errors SegmentedRingBuffer_init(SegmentedRingBuffer* dest, 
    size_t segment_capacity, 
    size_t opt_init_capacity);
Errors SegmentedRingBuffer_deinit(SegmentedRingBuffer* dest);
Errors SegmentedRingBuffer_push(SegmentedRingBuffer* dest, uint8_t value);
Errors SegmentedRingBuffer_pop(SegmentedRingBuffer* dest, uint8_t* value);
bool   SegmentedRingBuffer_is_empty(SegmentedRingBuffer* dest);

#endif  // INCLUDE_SEGMENTED_RING_BUFFER_H