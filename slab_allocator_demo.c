#include "slab_allocator_demo.h"
#include "slab_allocator.h"
#include "page_allocator.h"
#include "mem_demo_map.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

static void SlabAllocator_print_slab_list(Slab* list)
{
    Slab* current = list;
    size_t count = 0;
    while (current) {
        printf("used: %zu, slots addr: %p", current->used, current->slots_data);
        count++;
        current = current->next;
    }

    printf("\nslab count: %zu\n", count);
}

static void SlabAllocator_print_info(SlabAllocator* dest) 
{
    printf("object count: %d\n"
        "object size: %d\n"
        "object alignment: %d\n"
        "aligned object size: %d\n"
        "first object padding: %d\n\n",
        dest->object_count, dest->object_size, dest->object_alignment, dest->aligned_object_size, dest->first_object_padding);

    printf("empty slabs:\n");
    SlabAllocator_print_slab_list(dest->empty);
    printf("\n");

    printf("partial slabs:\n");
    SlabAllocator_print_slab_list(dest->partial);
    printf("\n");

    printf("full slabs:\n");
    SlabAllocator_print_slab_list(dest->full);
    printf("\n");
}

static void *allocate_f(void *dest)
{
    return SlabAllocator_allocate((SlabAllocator*) dest);
}

static Errors free_f(void *dest, void *address)
{
    (void)dest;
    return SlabAllocator_free(address);
}

void SlabAllocator_demo(void)
{
    size_t object_size = 0;
    size_t object_alignment = 0;
    size_t init_cache_capacity = 0;
    int command = 0;
    
    printf("Slab Allocator demo\n");

    printf("type object size:\n");
    scanf("%zu", &object_size);

    printf("type object alignment:\n");
    scanf("%zu", &object_alignment);

    printf("type init cache capacity:\n");
    scanf("%zu", &init_cache_capacity);

    Errors ret_code = OK;

    PageAllocator page_allocator;
    ret_code = PageAllocator_init(&page_allocator, 0, 0);
    if (ret_code != OK) {
        printf("error: %s\n", RetCode_get_str(ret_code));
        return;
    }

    SlabAllocator slab_allocator;
    SlabAllocator_init(&slab_allocator, &page_allocator, object_size, object_alignment, init_cache_capacity);
    if (ret_code != OK) {
        printf("error: %s\n", RetCode_get_str(ret_code));
        return;
    }

    MemMapDemo allocated_addresses_demo;
    MemMapDemo_init(&allocated_addresses_demo, &slab_allocator, allocate_f, free_f);

    do {
        SlabAllocator_print_info(&slab_allocator);

        printf("type command:\n"
            "0 - quit\n"
            "1 - allocate\n"
            "2 - deallocate\n"
            "3 - list\n");
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

            case 3: {
                MemMapDemo_list(&allocated_addresses_demo);
                break;
            }
        }

    } while (command != 0);

    MemMapDemo_deinit(&allocated_addresses_demo);
    SlabAllocator_deinit(&slab_allocator);
    PageAllocator_deinit(&page_allocator);
}
