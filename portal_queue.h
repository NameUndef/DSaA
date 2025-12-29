#ifndef INCLUDE_PORTAL_QUEUE_H
#define INCLUDE_PORTAL_QUEUE_H

#include "ret_code.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct _Segment {
    struct _Segment* next;
    struct _Segment* prev;
    uint8_t          data[];
} Segment;

typedef struct {
    Segment* free_segments;
    Segment* used_segments_ring;
    size_t   segment_capacity;
    size_t   head;
    size_t   tail;
    size_t   window_size;
    Segment* cur_segment_head;
    Segment* cur_segment_tail;
    Segment* segment_portal_src;
    Segment* segment_portal_dest;
    bool     full;
    bool     free_empty_segments;
    bool     reset_popped_values;
} PortalQueue;

Errors PortalQueue_init(
    PortalQueue* dest, 
    size_t segment_capacity, 
    size_t opt_init_capacity, 
    bool free_empty_segments,
    bool reset_popped_values);
Errors PortalQueue_deinit(PortalQueue* dest);
Errors PortalQueue_push(PortalQueue* dest, uint8_t value);
Errors PortalQueue_pop(PortalQueue* dest, uint8_t* value);
bool PortalQueue_is_full(PortalQueue* dest);
bool PortalQueue_is_empty(PortalQueue* dest);

#endif  // INCLUDE_PORTAL_QUEUE_H