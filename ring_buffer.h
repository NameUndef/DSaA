#ifndef INCLUDE_RING_BUFFER_H
#define INCLUDE_RING_BUFFER_H

#include <stdint.h>
#include <stdbool.h>
#include "ret_code.h"

typedef struct {
    uint8_t* buffer;
    size_t   capacity;
    size_t   head;
    size_t   tail;
    bool     full;
} RingBuffer;

Errors RingBuffer_init(RingBuffer* dest, size_t capacity);
Errors RingBuffer_deinit(RingBuffer* dest);
Errors RingBuffer_push(RingBuffer* buffer, uint8_t value);
Errors RingBuffer_pop(RingBuffer* buffer, uint8_t* value);
bool   RingBuffer_is_full(RingBuffer* buffer);

#endif  // INCLUDE_RING_BUFFER_H
