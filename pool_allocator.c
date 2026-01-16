#include "pool_allocator.h"
#include "page_manage.h"
#include "alignment.h"
#include <stdalign.h>

static void PA_fill_free_list(PoolAllocator* dest)
{
    uint8_t* current = dest->mem_start;
    dest->free_list = (void*)current;

    for (size_t i = 1; i < dest->capacity_count; i++) {
        uint8_t* next = current + dest->object_aligned_size;
        *(uint8_t**)current = next;
        current = next;
    }

    *((uint8_t**)current) = NULL;
}

Errors PoolAllocator_init(PoolAllocator* dest, size_t capacity_count, size_t object_size, size_t object_alignment)
{
    if (!dest) {
        return BAD_ARGUMENT_NULL_POINTER;
    }

    if (!capacity_count) {
        return BAD_ARGUMENT_ZERO;
    }

    if (!object_size) {
        return BAD_ARGUMENT_ZERO;
    }

    size_t page_size = get_page_size();

    if (object_alignment > page_size) {
        return BAD_ARGUMENT_OUT_OF_BOUNDS;
    }

    if (!is_power_of_2(object_alignment)) {
        return BAD_ARGUMENT_NOT_POWER_OF_2;
    }

    if (object_size < sizeof(void*)) {  // для хранения указателя в free list
        object_size = sizeof(void*);
    }

    if (object_alignment < alignof(void*)) {
        object_alignment = alignof(void*);
    }

    const size_t size_max = (size_t) -1;

    // проверка, будет ли выровненный размер вмещаться в адресное пространство
    const size_t aligned_low_border = get_aligned_low_border_value(object_size, object_alignment);

    if (object_size > object_alignment && object_size != aligned_low_border && aligned_low_border > size_max - object_alignment) {
        return BAD_ARGUMENT_OUT_OF_BOUNDS;
    }

    const size_t object_aligned_size = get_aligned_value(object_size, object_alignment);

    // учитывается хвостовой padding в конце массива
    if (capacity_count > 1 && size_max / capacity_count < object_aligned_size) {
        return BAD_ARGUMENT_OUT_OF_BOUNDS;
    }

    const size_t size = object_aligned_size * capacity_count;
    const size_t object_aligment_padding = object_aligned_size - object_size;

    // аллокация завязана на выравнивании адреса по странице
    // в противном случае пришлось бы добавить начальный запас для выравнивания
    void* data = allocate_page_sz(size);
    if (!data) {
        return ALLOC_ERROR;
    }

    dest->mem_start = data;
    dest->pool_size = size;
    dest->capacity_count = capacity_count;
    dest->object_size = object_size;
    dest->object_alignment_padding = object_aligment_padding;
    dest->object_aligned_size = object_aligned_size;
    dest->used = 0;
    dest->free_list = NULL;

    PA_fill_free_list(dest);

    return OK;
}

Errors PoolAllocator_deinit(PoolAllocator* dest)
{
    free_page_sz(dest->mem_start, dest->pool_size);

    dest->mem_start = NULL;
    dest->pool_size = 0;
    dest->capacity_count = 0;
    dest->object_size = 0;
    dest->object_alignment_padding = 0;
    dest->object_aligned_size = 0;
    dest->used = 0;
    dest->free_list = NULL;

    return OK;
}

void* PoolAllocator_allocate(PoolAllocator* dest)
{
    if (!dest) {
        return NULL;
    }

    if (!dest->free_list) {
        return NULL;
    }

    uint8_t* object = dest->free_list;
    dest->free_list = *(uint8_t**)object;   // последний будет NULL

    dest->used++;

    return (void*)object;
}

Errors PoolAllocator_free(PoolAllocator* dest, void* address)
{
    if (!dest) {
        return BAD_ARGUMENT_NULL_POINTER;
    }

    if (address < dest->mem_start || address >= dest->mem_start + dest->pool_size) {
        return BAD_ARGUMENT_OUT_OF_BOUNDS;
    }

    if (dest->used == 0) {
        return BAD_ARGUMENT_OUT_OF_BOUNDS;
    }

    const size_t object_offset = (size_t)((uint8_t*)address - dest->mem_start);
    const size_t aligned_offset = get_aligned_low_border_value(object_offset, dest->object_aligned_size);

    if (aligned_offset != object_offset) {
        return BAD_ARGUMENT_OUT_OF_BOUNDS;
    }

    *(uint8_t**)address = dest->free_list;
    dest->free_list = address;
    dest->used--;

    return OK;
}
