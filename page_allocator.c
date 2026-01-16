#include "page_allocator.h"
#include "page_manage.h"
#include "alignment.h"


#define CHUNK_PAGES_COUNT   512ULL
#define BITMAP_ELEMENT_SIZE 64ULL

static size_t get_0_bits_count_before_1(size_t value)
{
    size_t count = 0;
    for (count = 0; count < sizeof(value) << 3ULL; count++) {
        if ((value >> count) & 1ULL) {
            break;
        }
    }

    return count;
}

static Errors PA_pre_init(PageAllocator* dest, size_t chunk_pages_count) 
{
    if (!chunk_pages_count) {
        chunk_pages_count = CHUNK_PAGES_COUNT;
    }

    size_t page_size = get_page_size();
    size_t page_zero_bits_count = get_0_bits_count_before_1(page_size);
    size_t chunk_size = chunk_pages_count << page_zero_bits_count;
    size_t bitmap_array_size = (chunk_pages_count + (BITMAP_ELEMENT_SIZE - 1)) / BITMAP_ELEMENT_SIZE;
    size_t last_bitmap_size = chunk_pages_count % BITMAP_ELEMENT_SIZE;
    size_t headers_count_per_page = (page_size - sizeof(PA_ChunkHeaderPage)) / (sizeof(PA_ChunkHeader) + dest->bitmap_array_size);
    size_t nodes_count_per_page = (page_size - sizeof(PA_ChunkHeaderNodePage)) / sizeof(PA_ChunkHeaderNode);

    if (headers_count_per_page == 0) {
        return BAD_ARGUMENT_OUT_OF_BOUNDS;
    }

    dest->chunk_size = chunk_size;
    dest->chunk_page_count = chunk_pages_count;
    dest->bitmap_array_size = bitmap_array_size;
    dest->last_bitmap_size = last_bitmap_size;
    dest->headers_count_per_page = headers_count_per_page;
    dest->nodes_count_per_page = nodes_count_per_page;
    dest->cur_headers_count_in_page = chunk_pages_count;
    dest->cur_nodes_count_in_page = chunk_pages_count;
    dest->page_zero_bits_count = page_zero_bits_count;

    dest->header_pages = NULL;
    dest->node_pages = NULL;

    dest->last = NULL;
    dest->empty = NULL;
    dest->partial = NULL;
    dest->full = NULL;

    return OK;
}

static Errors PA_add_headers_page(PageAllocator* dest)
{
    void* page = allocate_page();
    if (!page) {
        return ALLOC_ERROR;
    }

    PA_ChunkHeaderPage* header_page = (PA_ChunkHeaderPage*) page;
    header_page->next = dest->header_pages;
    dest->header_pages = header_page;

    return OK;
}

static Errors PA_add_nodes_page(PageAllocator* dest)
{
    void* page = allocate_page();
    if (!page) {
        return ALLOC_ERROR;
    }

    PA_ChunkHeaderNodePage* header_page = (PA_ChunkHeaderNodePage*) page;
    header_page->next = dest->header_pages;
    dest->header_pages = header_page;

    return OK;
}

static void PA_remove_last_header_page(PageAllocator* dest)
{
    PA_ChunkHeaderPage* header_page = dest->header_pages;
    dest->header_pages = header_page->next;

    free_page((void*)header_page);
}

static Errors PA_add_chunk(PageAllocator* dest)
{
    Errors ret_code = OK;

    void* chunk = allocate_page_sz(dest->chunk_size);
    if (!chunk) {
        return ALLOC_ERROR;
    }

    if (dest->cur_headers_count_in_page == dest->headers_count_per_page) {
        ret_code = PA_add_headers_page(dest);
        if (ret_code != OK) {
            free_page_sz(chunk, dest->chunk_size);
            return ret_code;
        }
        dest->cur_headers_count_in_page = 0;
    }

    PA_ChunkHeader* header = dest->header_pages->headers + dest->cur_headers_count_in_page;
    dest->cur_headers_count_in_page++;

    if (dest->cur_nodes_count_in_page == dest->nodes_count_per_page) {
        ret_code = PA_add_nodes_page(dest);
        if (ret_code != OK) {
            if (dest->cur_headers_count_in_page == 1) {
                PA_remove_last_header_page(dest);
                dest->cur_headers_count_in_page = dest->headers_count_per_page;
                free_page_sz(chunk, dest->chunk_size);
            }
            return ret_code;
        }
        dest->cur_nodes_count_in_page = 0;
    }

    PA_ChunkHeaderNode* node = dest->node_pages->nodes + dest->cur_nodes_count_in_page;
    dest->cur_nodes_count_in_page++;

    // инициализируем header
    header->prev = NULL;
    header->next = dest->empty;
    header->used = 0;
    header->chunk_address = chunk;

    for (size_t i = 0; i < dest->bitmap_array_size; i++) {
        header->use_bitmap[i] = 0;
    }

    if (dest->empty) {
        dest->empty->prev = header;
    }

    dest->empty = header;
    
    // инициализируем node
    node->chunk_address = chunk;
    node->chunk_header = header;
}

static void PA_post_deinit(PageAllocator* dest) 
{
    dest->chunk_size = 0;
    dest->chunk_page_count = 0;
    dest->bitmap_array_size = 0;
    dest->last_bitmap_size = 0;
    dest->headers_count_per_page = 0;
    dest->nodes_count_per_page = 0;
    dest->cur_headers_count_in_page = 0;
    dest->cur_nodes_count_in_page = 0;
    dest->page_zero_bits_count = 0;

    dest->header_pages = NULL;
    dest->node_pages = NULL;

    dest->last = NULL;
    dest->empty = NULL;
    dest->partial = NULL;
    dest->full = NULL;
}

static void PA_free_chunks_list(PageAllocator* dest, PA_ChunkHeader* list)
{
    while (list) {
        free_page_sz(list->chunk_address, dest->chunk_size);
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

static void PA_free_chunks(PageAllocator* dest)
{
    PA_free_chunks_list(dest, dest->empty);
    PA_free_chunks_list(dest, dest->partial);
    PA_free_chunks_list(dest, dest->full);

    PA_free_headers_list(dest->header_pages);
    PA_free_nodes_list(dest->node_pages);
}

Errors PageAllocator_init(PageAllocator* dest, size_t chunk_pages_count, size_t init_chunk_count)
{
    if (!dest) {
        return BAD_ARGUMENT_NULL_POINTER;
    }

    Errors ret_code = PA_pre_init(dest, chunk_pages_count);
    if (ret_code != OK) {
        return ret_code;
    }

    if (!init_chunk_count) {
        init_chunk_count = 1;
    }

    for (size_t i = 0; i < init_chunk_count; i++) {
        ret_code = PA_add_chunk(dest);
        if (ret_code != OK) {
            PA_free_chunks(dest);
            PA_post_deinit(dest);
            return ret_code;
        }
    }

    return OK;
}

Errors PageAllocator_deinit(PageAllocator* dest)
{
    if (!dest) {
        return BAD_ARGUMENT_NULL_POINTER;
    }

    PA_free_chunks(dest);
    PA_post_deinit(dest);

    return OK;
}

static void* PA_allocate_page_from_chunk(PageAllocator* dest, PA_ChunkHeader* header)
{
    // Отсутствие пустых страниц - инвариант для данной функции. Они по-умолчанию должны быть
    size_t i = 0;
    size_t bit_idx = 0;

    for (i = 0; i < dest->bitmap_array_size; i++) {

        size_t bitmap_size = i == dest->bitmap_array_size - 1 ? dest->last_bitmap_size : BITMAP_ELEMENT_SIZE;

        for (bit_idx = 0; i < bitmap_size; bit_idx++) {
            if (!(header->use_bitmap[i] & (1 << bit_idx))) {
                break;   
            }
        }
    }

    header->used++;
    header->use_bitmap[i] |= (1 << bit_idx);

    size_t page_idx = i * BITMAP_ELEMENT_SIZE + bit_idx;
    return header->chunk_address + (page_idx * dest->chunk_size);
}

static void PA_move_first_chunk_to_first(PageAllocator* dest, PA_ChunkHeader** chunk_list_src, PA_ChunkHeader** list_dest) 
{
    PA_ChunkHeader* header = *chunk_list_src;
    if (header->next) {
        header->next->prev = NULL;
    }

    *chunk_list_src = header->next;

    header->next = *list_dest;
    *list_dest = header;
}

void* PageAllocator_allocate(PageAllocator* dest)
{
    if (!dest) {
        return NULL;
    }

    PA_ChunkHeader** header_list = NULL;

    if (dest->partial) {
        header_list = &dest->partial;

    } else if (dest->empty) {
        header_list = &dest->empty;

    } else {
        Errors ret_code = PA_add_chunk(dest);
        if (ret_code != OK) {
            return NULL;
        }

        header_list = &dest->empty;
    }

    PA_ChunkHeader* header = *header_list;
    void* data = PA_allocate_page_from_chunk(dest, header);

    if (header->used == dest->chunk_page_count) {
        PA_move_first_chunk_to_first(dest, header_list, &dest->full);
    } else if (header->used == 1) {
        PA_move_first_chunk_to_first(dest, header_list, &dest->partial);
    }

    return data;
}

static PA_ChunkHeaderNode* find_chunk_with_address_in_diapason(PageAllocator* dest, void* address)
{
    // сначала проверяем последний найденный чанк
    if (dest->last && address >= dest->last->chunk_address && address < dest->last->chunk_address + dest->chunk_size) {
        return dest->last;
    }

    PA_ChunkHeaderNodePage* page = dest->node_pages;
    PA_ChunkHeaderNode* tagret_node = NULL;

    size_t nodes_count = dest->cur_nodes_count_in_page;

    while (page) {

        for (size_t i = 0; i < nodes_count; i++) {
            if (address >= page->nodes[i].chunk_address && address < page->nodes[i].chunk_address + dest->chunk_size) {
                tagret_node = page->nodes + i;
                break;
            }
        }

        if (tagret_node) {
            break;
        }

        nodes_count = dest->nodes_count_per_page;
        page = page->next;
    }

    dest->last = tagret_node;

    return tagret_node;
}

static bool PA_free_page_from_chunk(PageAllocator* dest, PA_ChunkHeader* header, void* page)
{
    // адрес должен быть проверен и находиться в рамках чанка
    uintptr_t page_idx = (uintptr_t) (page - header->chunk_address) >> dest->page_zero_bits_count;
    size_t bitmap_idx = page_idx / BITMAP_ELEMENT_SIZE;
    size_t bit_idx = page_idx % BITMAP_ELEMENT_SIZE;

    if (!(header->use_bitmap[bitmap_idx] & (1 << bit_idx))) {
        return false;
    }

    header->use_bitmap[bitmap_idx] &= ~(1 << bit_idx);

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

Errors PageAllocator_free(PageAllocator* dest, void* page)
{
    if (!dest) {
        return BAD_ARGUMENT_NULL_POINTER;
    }

    if (!page) {
        return BAD_ARGUMENT_NULL_POINTER;
    }

    PA_ChunkHeaderNode* node = find_chunk_with_address_in_diapason(dest, page);
    if (!node) {
        return BAD_ARGUMENT_OUT_OF_BOUNDS;
    }

    PA_ChunkHeader* header = node->chunk_header;
    if (!header->used) {
        return ALREADY_FREE;
    }

    bool res = PA_free_page_from_chunk(dest, header, page);
    if (!res) {
        return ALREADY_FREE;
    }

    if (header->used == dest->chunk_page_count) {
        PA_move_chunk_to_first(header, &dest->full, (dest->chunk_page_count == 1)? &dest->empty : &dest->partial);
    } else if (header->used == 1) {
        PA_move_chunk_to_first(header, &dest->partial, &dest->empty);
    }

    header->used--;

    return OK;
}
