#ifndef INCLUDE_PAGE_ALLOCATOR_H
#define INCLUDE_PAGE_ALLOCATOR_H

#include "ret_code.h"
#include <stdlib.h>
#include <stdint.h>

typedef struct _PA_ChunkHeader {

    struct _PA_ChunkHeader* prev;
    struct _PA_ChunkHeader* next;
    size_t                  used;
    void*                   chunk_address;
    uint64_t                use_bitmap[];

} PA_ChunkHeader;

typedef struct _PA_ChunkHeaderNode {

    void*                   chunk_address;
    struct _PA_ChunkHeader* chunk_header;

} PA_ChunkHeaderNode;

typedef struct _PA_ChunkHeaderPage {

    struct _PA_ChunkHeaderPage* next;
    struct _PA_ChunkHeader      headers[];

} PA_ChunkHeaderPage;

typedef struct _PA_ChunkHeaderNodePage {

    struct _PA_ChunkHeaderNodePage* next;
    struct _PA_ChunkHeaderNode      nodes[];

} PA_ChunkHeaderNodePage;


typedef struct _PageCache {
    size_t                  page_size;
    size_t                  chunk_size;
    size_t                  chunk_page_count;
    size_t                  bitmap_array_size;
    size_t                  last_bitmap_size;
    size_t                  header_size;
    size_t                  headers_count_per_page;
    size_t                  nodes_count_per_page;
    size_t                  cur_headers_count_in_page;
    size_t                  cur_nodes_count_in_page;
    size_t                  page_zero_bits_count;

    PA_ChunkHeaderPage*     header_pages;
    PA_ChunkHeaderNodePage* node_pages;

    PA_ChunkHeaderNode*     last;
    PA_ChunkHeader*         empty;
    PA_ChunkHeader*         partial;
    PA_ChunkHeader*         full;

} PageCache;

typedef PageCache PageAllocator;

int PageAllocator_init(PageAllocator* obj, size_t chunk_pages_count, size_t init_chunk_count);
int PageAllocator_deinit(PageAllocator* obj);

void* PageAllocator_allocate_rc(PageAllocator* obj, int *ret_code);
void* PageAllocator_allocate(PageAllocator* obj);
int PageAllocator_free(PageAllocator* obj, void* page);

#endif  // INCLUDE_PAGE_ALLOCATOR_H