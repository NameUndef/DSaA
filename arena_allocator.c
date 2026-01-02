#include "arena_allocator.h"

Errors ArenaAllocator_init(ArenaAllocator* dest, size_t capacity)
{
    if (!dest) {
        return BAD_ARGUMENT_NULL_POINTER;
    }

    if (!capacity) {
        return BAD_ARGUMENT_ZERO;
    }

    uint8_t* data = (uint8_t*) malloc(capacity);
    if (!data) {
        return ALLOC_ERROR;
    }

    dest->data = data;
    dest->capacity = capacity;
    dest->offset = 0;

    return OK;
}

Errors ArenaAllocator_deinit(ArenaAllocator* dest)
{
    if (!dest) {
        return BAD_ARGUMENT_NULL_POINTER;
    }

    free(dest->data);
    dest->offset = 0;
    dest->capacity = 0;

    return OK;
}

void* ArenaAllocator_allocate(ArenaAllocator* dest, size_t size, size_t align)
{
    if (!dest) {
        return NULL;
    }

    if (!size) {
        return NULL;
    }

    // проверка на степень двойки
    if (!align || align & (align - 1)) {
        return NULL;
    }

    // получаем выровненный оффсет
    size_t aligned = (dest->offset + align - 1) & ~(align - 1);

    // проверка на переполнение
    if (aligned > dest->capacity || size > dest->capacity - aligned) {
        return NULL;
    }

    dest->offset = aligned + size;

    return dest->data + aligned;
}

Errors ArenaAllocator_reset(ArenaAllocator* dest)
{
    if (!dest) {
        return BAD_ARGUMENT_NULL_POINTER;
    }

    dest->offset = 0;

    return OK;
}
