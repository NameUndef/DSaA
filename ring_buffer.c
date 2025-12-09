#include "ring_buffer.h"
#include <stdlib.h>

Errors RingBuffer_init(RingBuffer* dest, size_t capacity)
{
    if (!dest) {
        return BAD_ARGUMENT_NULL_POINTER;
    }

    if (!capacity) {
        return BAD_ARGUMENT_ZERO;
    }

    uint8_t* buffer = (uint8_t*) malloc(capacity);
    if (!buffer) {
        return ALLOC_ERROR;
    }

    dest->buffer = buffer;
    dest->capacity = capacity;
    dest->head = 0;
    dest->tail = 0;
    dest->full = false;

    return OK;
}

Errors RingBuffer_deinit(RingBuffer* dest) 
{
    if (!dest) {
        return BAD_ARGUMENT_NULL_POINTER;
    }

    free(dest->buffer);

    dest->buffer = NULL;
    dest->capacity = 0;
    dest->head = 0;
    dest->tail = 0;
    dest->full = false;

    return OK;
}

Errors RingBuffer_push(RingBuffer* buffer, uint8_t value)
{
    if (!buffer) {
        return BAD_ARGUMENT_NULL_POINTER;
    }

    if (buffer->full) {
        return FULL_BUFFER;
    }

    buffer->buffer[buffer->head] = value;
    buffer->head = (buffer->head + 1) % buffer->capacity;
    buffer->full = (buffer->head == buffer->tail);

    return OK;
}

Errors RingBuffer_pop(RingBuffer* buffer, uint8_t* value) 
{
    if (!buffer) {
        return BAD_ARGUMENT_NULL_POINTER;
    }

    if (!value) {
        return BAD_ARGUMENT_NULL_POINTER;
    }

    if (buffer->head == buffer->tail && !buffer->full) {
        return EMPTY_BUFFER;
    }

    *value = buffer->buffer[buffer->tail];
    buffer->tail = (buffer->tail + 1) % buffer->capacity;
    buffer->full = false;

    return OK;
}

bool RingBuffer_is_full(RingBuffer* buffer) 
{
    return buffer && buffer->full;
}
