#include "page_allocator.h"
#include "mem_demo_map.h"
#include "ret_code.h"
#include <stdio.h>

static void *allocate_f(void* dest)
{
    return PageAllocator_allocate((PageAllocator*) dest);
}

static Errors free_f(void* obj, void* page)
{
    return (Errors) PageAllocator_free((PageAllocator*) obj, page);
}

void PageAllocator_print_info(PageAllocator* obj)
{
    printf("nodes in page: %zu\n", obj->cur_nodes_count_in_page);
    printf("headers in page: %zu\n", obj->cur_headers_count_in_page);
    printf("pages in chunk: %zu\n", obj->chunk_page_count);
    
    PA_ChunkHeader *header = obj->partial;
    printf("partial:\n");
    while (header) {
        printf("chunk address: %p, used: %zu\n", header->chunk_address, header->used);
        header = header->next;
    }

    header = obj->full;
    printf("full:\n");
    while (header) {
        printf("chunk address: %p\n", header->chunk_address);
        header = header->next;
    }
}

void PageAllocator_demo(void)
{
    size_t chunk_page_count = 0;
    size_t init_chunk_count = 0;
    int command = 0;

    printf("PageAllocator demo\n");
    printf("type pages count in chunk, 0 sets default (512):\n");
    scanf("%zu", &chunk_page_count);

    printf("type init chunk count, 0 sets default (1):\n");
    scanf("%zu", &init_chunk_count);

    Errors ret_code = OK;
    PageAllocator allocator;
    ret_code = PageAllocator_init(&allocator, chunk_page_count, init_chunk_count);
    if (ret_code != OK) {
        printf("error: %s\n", RetCode_get_str(ret_code));
        return;
    }

    MemMapDemo allocated_addresses_demo;
    MemMapDemo_init(&allocated_addresses_demo, &allocator, allocate_f, free_f);

    do {

        PageAllocator_print_info(&allocator);

        printf("type command:\n"
            "0 - quit\n"
            "1 - allocate\n"
            "2 - deallocate\n");
        scanf("%d", &command);
        
        switch (command) {

            default: {
                printf("unknown command\n");
                break;
            }

            case 0: {
                break;
            }

            case 1: {
                MemMapDemo_allocate_dialogue(&allocated_addresses_demo);
                break;
            }

            case 2: {
                MemMapDemo_free_dialogue(&allocated_addresses_demo);
                break;
            }
            
        }
    } while (command != 0);

    MemMapDemo_deinit(&allocated_addresses_demo);
    PageAllocator_deinit(&allocator);
}
