#include "pool_allocator.h"
#include "ret_code.h"
#include "mem_demo_map.h"
#include <stdio.h>

void PageAllocator_demo(void)
{
    size_t object_count = 0;
    size_t object_size = 0;
    size_t object_alignment = 0;
    int command = 0;

    printf("PoolAllocator demo\n");
    printf("type object count:\n");
    scanf("%zu", &object_count);

    printf("type object size:\n");
    scanf("%zu", &object_size);

    printf("type object alignment:\n");
    scanf("%zu", &object_alignment);

    Errors ret_code = OK;
    PoolAllocator allocator;
    ret_code = PoolAllocator_init(&allocator, object_count, object_size, object_alignment);
    if (ret_code != OK) {
        printf("error: %s\n", RetCode_get_str(ret_code));
        return;
    }

    MemMapDemo allocated_addresses_demo;
    MemMapDemo_init(&allocated_addresses_demo, &allocator, PoolAllocator_allocate, PoolAllocator_free);

    do {
        printf("--------\n");
        printf("Used: %zu, free: %zu\n", PoolAllocator_get_used(&allocator), PoolAllocator_get_free(&allocator));

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
    PoolAllocator_deinit(&allocator);
}
