#include "arena_allocator_demo.h"
#include "arena_allocator.h"
#include <stdio.h>

static void ArenaAllocator_print_info(ArenaAllocator* dest)
{
    printf("size: %zu\noffset: %zu\n", dest->capacity, dest->offset);

    for (size_t i = 0; i < dest->offset; i++) {
        printf("%03d|", dest->data[i]);
    }
    printf("\n");
}

static void ArenaAllocator_fill_by_zero(ArenaAllocator* dest)
{
    for (size_t i = 0; i < dest->capacity; i++) {
        dest->data[i] = 0;
    }
}

void ArenaAllocator_demo(void)
{
    size_t capacity = 0;
    Errors ret_code = OK;
    int command = 0;

    ArenaAllocator allocator;

    printf("ArenaAllocator demo\n");
    printf("type capacity:\n");
    scanf("%zu", &capacity);

    ret_code = ArenaAllocator_init(&allocator, capacity);
    if (ret_code != OK)
    {
        printf("error: %s\n", RetCode_get_str(ret_code));
        return;
    }

    ArenaAllocator_fill_by_zero(&allocator);

    do
    {
        printf("--------\n");
        ArenaAllocator_print_info(&allocator);
        printf("type command:\n"
            "0 - quit\n"
            "1 - allocate\n"
            "2 - reset\n");
        scanf("%d", &command);

        switch (command)
        {
            default: {
                printf("unknown command\n");
                break;
            }

            case 0: {
                break;
            }

            case 1: {
                printf("type size:\n");
                size_t size = 0;
                scanf("%zu", &size);

                printf("type alignment:\n");
                size_t align = 0;
                scanf("%zu", &align);

                void* address = ArenaAllocator_allocate(&allocator, size, align);
                if (address == NULL) {
                    printf("error: can't allocate memory\n");
                    break;
                }

                printf("address: %p\n", address);
                uint8_t* data = (uint8_t*)address;
                for (size_t i = 0; i < size; i++) {
                    data[i] = (uint8_t)(i + 1);
                }

                break;
            }

            case 2: {
                printf("reset arena\n");
                ArenaAllocator_reset(&allocator);
                ArenaAllocator_fill_by_zero(&allocator);
                break;
            }

        }

    } while (command != 0);

    ArenaAllocator_deinit(&allocator);
}
