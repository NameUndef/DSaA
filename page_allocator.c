#include "page_allocator.h"
#include "page_manage.h"
#include "alignment.h"

#include <stdio.h>
#include "utils.h"

#define CHUNK_PAGES_COUNT   512ULL
#define BITMAP_ELEMENT_SIZE 64ULL
#define BITMAP_ELEMENT_BYTES 8ULL

void print_bitmaps(PageAllocator *obj)
{
    for (size_t i = 0; i < obj->cur_nodes_count_in_page; i++) {
        PA_ChunkHeader* header = obj->node_pages->nodes[i].chunk_header;
        
        printf("address: %p\n", header->chunk_address);
        for (size_t bitmap = 0; bitmap < obj->bitmap_array_size; bitmap++) {
            printf("%p: ", &header->use_bitmap[bitmap]);
            print_u64_bits(header->use_bitmap[bitmap]);
        }
    }

}

static int PA_pre_init(PageAllocator* obj, size_t chunk_pages_count) 
{
    if (!chunk_pages_count) {
        chunk_pages_count = CHUNK_PAGES_COUNT;
    }

    size_t page_size = get_page_size();
    size_t page_zero_bits_count = get_0_bits_count_before_1(page_size);
    size_t chunk_size = chunk_pages_count << page_zero_bits_count;
    size_t bitmap_array_size = (chunk_pages_count + (BITMAP_ELEMENT_SIZE - 1)) / BITMAP_ELEMENT_SIZE;
    size_t last_bitmap_size = chunk_pages_count % BITMAP_ELEMENT_SIZE;

    if (last_bitmap_size) {
        bitmap_array_size++;
    } else {
        last_bitmap_size = BITMAP_ELEMENT_SIZE;
    }

    size_t header_size = sizeof(PA_ChunkHeader) + BITMAP_ELEMENT_BYTES * bitmap_array_size;

    size_t headers_count_per_page = (page_size - sizeof(PA_ChunkHeaderPage)) / header_size;
    size_t nodes_count_per_page = (page_size - sizeof(PA_ChunkHeaderNodePage)) / sizeof(PA_ChunkHeaderNode);

    if (headers_count_per_page == 0) {
        return BAD_ARGUMENT_OUT_OF_BOUNDS;
    }

    obj->page_size = page_size;
    obj->chunk_size = chunk_size;
    obj->chunk_page_count = chunk_pages_count;
    obj->bitmap_array_size = bitmap_array_size;
    obj->last_bitmap_size = last_bitmap_size;
    obj->header_size = header_size;
    obj->headers_count_per_page = headers_count_per_page;
    obj->nodes_count_per_page = nodes_count_per_page;
    obj->cur_headers_count_in_page = headers_count_per_page;
    obj->cur_nodes_count_in_page = nodes_count_per_page;
    obj->page_zero_bits_count = page_zero_bits_count;

    obj->header_pages = NULL;
    obj->node_pages = NULL;

    obj->last = NULL;
    obj->empty = NULL;
    obj->partial = NULL;
    obj->full = NULL;

    return OK;
}

static int PA_add_headers_page(PageAllocator* obj)
{
    void* page = allocate_page();

    if (!page) {
        return ALLOC_ERROR;
    }

    PA_ChunkHeaderPage* header_page = (PA_ChunkHeaderPage*) page;
    header_page->next = obj->header_pages;
    obj->header_pages = header_page;

    return OK;
}

static int PA_add_nodes_page(PageAllocator* obj)
{
    void* page = allocate_page();
    if (!page) {
        return ALLOC_ERROR;
    }

    PA_ChunkHeaderNodePage* node_page = (PA_ChunkHeaderNodePage*) page;
    node_page->next = obj->node_pages;
    obj->node_pages = node_page;

    return OK;
}

static void PA_remove_last_header_page(PageAllocator* obj)
{
    PA_ChunkHeaderPage* header_page = obj->header_pages;
    obj->header_pages = header_page->next;

    free_page((void*)header_page);
}

static int PA_add_chunk(PageAllocator* obj)
{
    int ret_code = OK;

    void* chunk = allocate_page_sz(obj->chunk_size);
    if (!chunk) {
        return ALLOC_ERROR;
    }

    if (obj->cur_headers_count_in_page == obj->headers_count_per_page) {
        ret_code = PA_add_headers_page(obj);
        if (ret_code != OK) {
            free_page_sz(chunk, obj->chunk_size);
            return ret_code;
        }
        obj->cur_headers_count_in_page = 0;
    }

    PA_ChunkHeader* header = (PA_ChunkHeader*)((char*)obj->header_pages->headers + obj->header_size * obj->cur_headers_count_in_page);
    obj->cur_headers_count_in_page++;

    if (obj->cur_nodes_count_in_page == obj->nodes_count_per_page) {
        ret_code = PA_add_nodes_page(obj);
        if (ret_code != OK) {
            if (obj->cur_headers_count_in_page == 1) {
                PA_remove_last_header_page(obj);
                obj->cur_headers_count_in_page = obj->headers_count_per_page;
                free_page_sz(chunk, obj->chunk_size);
            }
            return ret_code;
        }
        obj->cur_nodes_count_in_page = 0;
    }

    PA_ChunkHeaderNode* node = obj->node_pages->nodes + obj->cur_nodes_count_in_page;
    obj->cur_nodes_count_in_page++;

    // printf("%p - %p - %p\n", obj->header_pages->headers, header, obj->header_pages + 4096);

    // printf("%llu, %llu, %llu\n", obj->cur_headers_count_in_page, obj->headers_count_per_page, obj->header_size);

    // printf("%llu, %llu\n", obj->cur_nodes_count_in_page, obj->nodes_count_per_page);

    // инициализируем header
    header->prev = NULL;
    header->next = obj->empty;
    header->used = 0;
    header->chunk_address = chunk;

    for (size_t i = 0; i < obj->bitmap_array_size; i++) {
        header->use_bitmap[i] = 0;
    }

    if (obj->empty) {
        obj->empty->prev = header;
    }

    obj->empty = header;
    
    // инициализируем node
    node->chunk_address = chunk;
    node->chunk_header = header;
    
    return OK;
}

static void PA_post_deinit(PageAllocator* obj) 
{
    obj->page_size = 0;
    obj->chunk_size = 0;
    obj->chunk_page_count = 0;
    obj->bitmap_array_size = 0;
    obj->last_bitmap_size = 0;
    obj->header_size = 0;
    obj->headers_count_per_page = 0;
    obj->nodes_count_per_page = 0;
    obj->cur_headers_count_in_page = 0;
    obj->cur_nodes_count_in_page = 0;
    obj->page_zero_bits_count = 0;

    obj->header_pages = NULL;
    obj->node_pages = NULL;

    obj->last = NULL;
    obj->empty = NULL;
    obj->partial = NULL;
    obj->full = NULL;
}

static void PA_free_chunks_list(PageAllocator* obj, PA_ChunkHeader* list)
{
    while (list) {
        free_page_sz(list->chunk_address, obj->chunk_size);
        list = list->next;
    }
}

static void PA_free_headers_list(PA_ChunkHeaderPage* list)
{
    while (list) {
        PA_ChunkHeaderPage* next = list->next;
        free_page(list);
        list = next;
    }
}

static void PA_free_nodes_list(PA_ChunkHeaderNodePage* list)
{
    while (list) {
        PA_ChunkHeaderNodePage* next = list->next;
        free_page(list);
        list = next;
    }
}

static void PA_free_chunks(PageAllocator* obj)
{
    PA_free_chunks_list(obj, obj->empty);
    PA_free_chunks_list(obj, obj->partial);
    PA_free_chunks_list(obj, obj->full);

    PA_free_headers_list(obj->header_pages);
    PA_free_nodes_list(obj->node_pages);
}

int PageAllocator_init(PageAllocator* obj, size_t chunk_pages_count, size_t init_chunk_count)
{
    if (!obj) {
        return BAD_ARGUMENT_NULL_POINTER;
    }

    int ret_code = PA_pre_init(obj, chunk_pages_count);
    if (ret_code != OK) {
        return ret_code;
    }

    if (!init_chunk_count) {
        init_chunk_count = 1;
    }

    for (size_t i = 0; i < init_chunk_count; i++) {
        ret_code = PA_add_chunk(obj);
        if (ret_code != OK) {
            PA_free_chunks(obj);
            PA_post_deinit(obj);
            return ret_code;
        }
    }

    return OK;
}

int PageAllocator_deinit(PageAllocator* obj)
{
    if (!obj) {
        return BAD_ARGUMENT_NULL_POINTER;
    }

    PA_free_chunks(obj);
    PA_post_deinit(obj);

    return OK;
}

static void* PA_allocate_page_from_chunk(PageAllocator* obj, PA_ChunkHeader* header)
{
    // Отсутствие пустых страниц - инвариант для данной функции. Они по-умолчанию должны быть
    size_t i = 0;
    size_t bit_idx = 0;

    bool free_page_found = false;
    for (i = 0; i < obj->bitmap_array_size; i++) {

        size_t bitmap_size = i == obj->bitmap_array_size - 1 ? obj->last_bitmap_size : BITMAP_ELEMENT_SIZE;

        for (bit_idx = 0; bit_idx < bitmap_size; bit_idx++) {

            if (!(header->use_bitmap[i] & (1ULL << bit_idx))) {
                free_page_found = true;
                break;   
            }
        }

        if (free_page_found) {
            break;
        }
    }

    header->used++;
    header->use_bitmap[i] |= (1ULL << bit_idx);

    // printf("alloc: %p, %llu, %llu, used: %llu, %llu, ", header->chunk_address, i, bit_idx, header->used, obj->chunk_page_count);
    // printf("%p: ", &header->use_bitmap[i]);
    // print_u64_bits(header->use_bitmap[i]);
    size_t page_idx = i * BITMAP_ELEMENT_SIZE + bit_idx;
    return header->chunk_address + (page_idx * obj->page_size);
}


static void PA_move_first_chunk_to_first(PageAllocator* obj, PA_ChunkHeader** chunk_list_src, PA_ChunkHeader** list_dest) 
{
    PA_ChunkHeader* header = *chunk_list_src;
    if (header->next) {
        header->next->prev = NULL;
    }

    *chunk_list_src = header->next;

    header->next = *list_dest;
    *list_dest = header;
}

void* PageAllocator_allocate_rc(PageAllocator* obj, int *ret_code)
{
    if (!obj) {
        if (ret_code != NULL) {
            *ret_code = BAD_ARGUMENT_NULL_POINTER;
        }
        return NULL;
    }

    PA_ChunkHeader** header_list = NULL;

    if (obj->partial) {
        header_list = &obj->partial;

    } else if (obj->empty) {
        header_list = &obj->empty;

    } else {
        int ret_code_inner = PA_add_chunk(obj);
        if (ret_code_inner != OK) {
            if (ret_code != NULL) {
                *ret_code = ret_code_inner;
            }
            return NULL;
        }

        header_list = &obj->empty;
    }

    PA_ChunkHeader* header = *header_list;
    void* data = PA_allocate_page_from_chunk(obj, header);

    if (header->used == obj->chunk_page_count) {
        PA_move_first_chunk_to_first(obj, header_list, &obj->full);
    } else if (header->used == 1) {
        PA_move_first_chunk_to_first(obj, header_list, &obj->partial);
    }

    return data;
}

void* PageAllocator_allocate(PageAllocator* obj)
{
    return PageAllocator_allocate_rc(obj, NULL);
}

static PA_ChunkHeaderNode* find_chunk_with_address_in_range(PageAllocator* obj, void* address)
{
    // сначала проверяем последний найденный чанк
    if (obj->last && address >= obj->last->chunk_address && address < obj->last->chunk_address + obj->chunk_size) {
        return obj->last;
    }

    PA_ChunkHeaderNodePage* page = obj->node_pages;
    PA_ChunkHeaderNode* tagret_node = NULL;

    size_t nodes_count = obj->cur_nodes_count_in_page;

    while (page) {

        for (size_t i = 0; i < nodes_count; i++) {

            if (address >= page->nodes[i].chunk_address && address < page->nodes[i].chunk_address + obj->chunk_size) {
                tagret_node = page->nodes + i;
                break;
            }
        }

        if (tagret_node) {
            break;
        }

        nodes_count = obj->nodes_count_per_page;
        page = page->next;
    }

    obj->last = tagret_node;

    return tagret_node;
}

static bool PA_free_page_from_chunk(PageAllocator* obj, PA_ChunkHeader* header, void* page)
{
    // адрес должен быть проверен и находиться в рамках чанка
    uintptr_t page_idx = (uintptr_t) (page - header->chunk_address) >> obj->page_zero_bits_count;
    size_t bitmap_idx = page_idx / BITMAP_ELEMENT_SIZE;
    size_t bit_idx = page_idx % BITMAP_ELEMENT_SIZE;

    // printf("free: %p, %llu, %llu ", header->chunk_address, bitmap_idx, bit_idx);
    // printf("%llu: ", header->used);
    // print_u64_bits(header->use_bitmap[bitmap_idx]);

    if (!(header->use_bitmap[bitmap_idx] & (1ULL << bit_idx))) {
        return false;
    }

    header->use_bitmap[bitmap_idx] &= ~(1ULL << bit_idx);

    return true;
}

static void PA_move_chunk_to_first(PA_ChunkHeader* header, PA_ChunkHeader** list_src, PA_ChunkHeader** list_dest)
{
    // вынимаем slab из списка
    if (header->next) {
        header->next->prev = header->prev;
    }

    if (header->prev) {
        header->prev->next = header->next;
    } {
        // slab первый в исходном списке
        *list_src = header->next;
    }

    // устанавливаем slab в начало целевого списка
    header->prev = NULL;
    header->next = *list_dest;
    *list_dest = header;
}

int PageAllocator_free(PageAllocator* obj, void* page)
{
    if (!obj) {
        return BAD_ARGUMENT_NULL_POINTER;
    }

    if (!page) {
        return BAD_ARGUMENT_NULL_POINTER;
    }

    PA_ChunkHeaderNode* node = find_chunk_with_address_in_range(obj, page);
    if (!node) {
        return BAD_ARGUMENT_OUT_OF_BOUNDS;
    }

    PA_ChunkHeader* header = node->chunk_header;
    if (!header->used) {
        return ALREADY_FREE;
    }

    bool res = PA_free_page_from_chunk(obj, header, page);
    if (!res) {
        return ALREADY_FREE;
    }

    if (header->used == obj->chunk_page_count) {
        PA_move_chunk_to_first(header, &obj->full, (obj->chunk_page_count == 1)? &obj->empty : &obj->partial);
    } else if (header->used == 1) {
        PA_move_chunk_to_first(header, &obj->partial, &obj->empty);
    }

    header->used--;

    return OK;
}
