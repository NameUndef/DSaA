#include "slab_allocator.h"
#include "alignment.h"
#include <stdbool.h>
#include <stdalign.h>

static Errors SA_add_slab(SlabAllocator *dest)
{
    void* page = PageAllocator_allocate(dest->page_allocator);

    if (!page) {
        return ALLOC_ERROR;
    }

    Slab* slab = (Slab*) page;

    // создание free list
    uint8_t* current = slab->slots_data + dest->first_object_padding;
    slab->free_list = current;

    for (size_t i = 1; i < dest->object_count; i++) {
        uint8_t* next = current + dest->aligned_object_size;
        *(uint8_t**)current = next;
        current = next;
    }

    *((uint8_t**)current) = NULL;

    slab->used = 0;

    // внедрение в список кэша
    slab->prev = NULL;
    slab->next = dest->empty;

    Slab* second_slab = dest->empty;
    if (second_slab) {
        second_slab->prev = slab;
    }

    dest->empty = slab;

    slab->cache = dest;

    return OK;
}

static void SA_remove_slab_list(SlabAllocator *dest, Slab* list) 
{
    Slab* current = list;

    while (current) {
        Slab* next = current->next;
        PageAllocator_free(dest->page_allocator, current);
        current = next;
    }
}

static void SA_remove_all_slabs(SlabAllocator *dest)
{
    SA_remove_slab_list(dest, dest->empty);
    SA_remove_slab_list(dest, dest->partial);
    SA_remove_slab_list(dest, dest->full);

    dest->empty = NULL;
    dest->partial = NULL;
    dest->full = NULL;
}

Errors SlabAllocator_init(
    SlabAllocator *dest, 
    PageAllocator* page_allocator,
    size_t object_size, 
    size_t object_alignment, 
    size_t init_cache_capacity)
{
    if (!dest) {
        return BAD_ARGUMENT_NULL_POINTER;
    }

    if (!object_size) {
        return BAD_ARGUMENT_ZERO;
    }

    if (!page_allocator) {
        return BAD_ARGUMENT_NULL_POINTER;
    }

    if (!is_power_of_2(object_alignment)) {
        return BAD_ARGUMENT_NOT_POWER_OF_2;
    }

    if (!init_cache_capacity) {
        init_cache_capacity = 1;
    }

    if (object_size < sizeof(void*)) {  // для хранения указателя в free list
        object_size = sizeof(void*);
    }

    if (object_alignment < alignof(void*)) {
        object_alignment = alignof(void*);
    }

    size_t page_size = get_page_size();
    size_t slab_header_size = sizeof(Slab);

    if (object_alignment >= page_size) {
        return BAD_ARGUMENT_OUT_OF_BOUNDS;
    }

    if (object_size >= page_size) {
        return BAD_ARGUMENT_OUT_OF_BOUNDS;
    }

    // определить и проверить сдвиг первого элемента
    size_t first_object_offset = get_aligned_value(slab_header_size, object_alignment);

    if (first_object_offset >= page_size || (page_size - first_object_offset) < object_size) {
        return BAD_ARGUMENT_OUT_OF_BOUNDS;
    }

    size_t aligned_object_size = get_aligned_value(object_size, object_alignment);

    // один объект точно вмещается
    size_t object_count = 1 + ((page_size - first_object_offset - object_size) / aligned_object_size);

    // инициализировать кэш
    dest->page_allocator = page_allocator;
    dest->object_count = object_count;
    dest->object_size = object_size;
    dest->object_alignment = object_alignment;
    dest->first_object_padding = first_object_offset - sizeof(Slab);
    dest->aligned_object_size = aligned_object_size;
    dest->empty = NULL;
    dest->partial = NULL;
    dest->full = NULL;

    // добавить slab'ы
    for (size_t i = 0; i < init_cache_capacity; i++) {
        Errors ret_code = SA_add_slab(dest);
        if (ret_code != OK) {
            SA_remove_all_slabs(dest);
            return ret_code;
        }
    }

    return OK;
}

Errors SlabAllocator_deinit(SlabAllocator *dest)
{
    if (!dest) {
        return BAD_ARGUMENT_NULL_POINTER;
    }

    SA_remove_all_slabs(dest);

    dest->object_count = 0;
    dest->object_size = 0;
    dest->object_alignment = 0;
    dest->first_object_padding = 0;
    dest->aligned_object_size = 0;
    
    return OK;
}

static void* SA_allocate_from_slab(SlabAllocator *dest, Slab* slab)
{
    // берем первый свободный объект
    uint8_t* object = slab->free_list;
    // указываем следующий элемент в free list, он может быть и последним (NULL)
    slab->free_list = *(uint8_t**)object;

    slab->used++;

    return (void*)object;
}

static void SA_move_first_slab_to_first(SlabAllocator *dest, Slab** slab_list_src, Slab** slab_list_dest)
{
    Slab* slab = *slab_list_src;

    if (slab->next) {
        slab->next->prev = NULL;
    }

    *slab_list_src = slab->next;    // NULL, если slab последний

    // slab->prev уже NULL
    slab->next = *slab_list_dest;        // NULL, если в full никого не было
    *slab_list_dest = slab;
}

void* SlabAllocator_allocate(SlabAllocator *dest)
{
    if (!dest) {
        return NULL;
    }

    Slab** slab_list_ptr;

    if (dest->partial) {
        slab_list_ptr = &dest->partial;

    } else if (dest->empty) {
        slab_list_ptr = &dest->empty;

    } else {
        Errors ret_code = SA_add_slab(dest);
        if (ret_code != OK) {
            return NULL;
        }

        slab_list_ptr = &dest->empty;
    }

    Slab* slab = *slab_list_ptr;
    void* data = SA_allocate_from_slab(dest, slab);

    if (slab->used == dest->object_count) {
        SA_move_first_slab_to_first(dest, slab_list_ptr, &dest->full);
    } else if (slab->used == 1) {
        SA_move_first_slab_to_first(dest, slab_list_ptr, &dest->partial);
    }

    return data;
}

static void SA_move_slab_to_first(Slab* slab, Slab** src_list, Slab** dest_list)
{
    // вынимаем slab из списка
    if (slab->next) {
        slab->next->prev = slab->prev;
    }

    if (slab->prev) {
        slab->prev->next = slab->next;
    } {
        // slab первый в исходном списке
        *src_list = slab->next;
    }

    // устанавливаем slab в начало целевого списка
    slab->prev = NULL;
    slab->next = *dest_list;
    *dest_list = slab;
}

Errors SlabAllocator_free(void *data)
{
    if (!data) {
        return BAD_ARGUMENT_NULL_POINTER;
    }

    size_t page_size = get_page_size();
    uintptr_t page_addr = (uintptr_t)data & ~(uintptr_t)(page_size - 1);

    Slab* slab = (Slab*) page_addr;

    if (slab->used == 0) {
        return BAD_ARGUMENT_OUT_OF_BOUNDS;
    }

    uint8_t** list_object_ptr = (uint8_t**) data;

    // установим следующий указатель в освободившийся объект,
    // и укажем его как первый элемент в free list
    *list_object_ptr = slab->free_list;
    slab->free_list = (uint8_t*) list_object_ptr;

    SlabAllocator* cache = slab->cache;

    // из full в partial, used == count, count > 1
    // из full в empty, used == count, count == 1
    // из partial в empty, used < count, used == 1
    // из partial в partial, used < count, used > 1 (не перемещать)
    if (slab->used == cache->object_count) {
        SA_move_slab_to_first(slab, &cache->full, (cache->object_count == 1)? &cache->empty : &cache->partial);
    } else if (slab->used == 1) {
        SA_move_slab_to_first(slab, &cache->partial, &cache->empty);
    }

    slab->used--;

    return OK;
}
