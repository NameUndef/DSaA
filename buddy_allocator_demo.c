#include "buddy_allocator.h"
#include "mem_demo_map.h"
#include "ret_code.h"

#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>

static void *allocate_f(void* dest, size_t size)
{
    return BuddyAllocator_allocate((BuddyAllocator*) dest, size);
}

static Errors free_f(void* obj, void* address)
{
    return (Errors) BuddyAllocator_free((BuddyAllocator*) obj, address);
}

static void BA_print_cur_node(size_t tab_len, BuddyAllocator *obj, size_t cur_idx)
{
    for (size_t i = 0; i < tab_len; i++) {
        putchar(' ');
    }

    size_t shift = (char) obj->nodes_tree[cur_idx].max_size_shift;
    printf("%zu: %zu (%zu)\n", cur_idx, 1ULL << shift, shift);

    size_t left_child_idx = 2 * cur_idx + 1;
    size_t right_child_idx = left_child_idx + 1;

    if (right_child_idx >= obj->nodes_count) {
        return;
    }

    tab_len += 4;
    BA_print_cur_node(tab_len, obj, left_child_idx);
    BA_print_cur_node(tab_len, obj, right_child_idx);
}

void BuddyAllocator_print_info(BuddyAllocator *obj)
{
    printf("size: %zu, nodes count: %zu, minimum alloc size: %zu\nnode tree:\n", obj->data_size, obj->nodes_count, obj->minimum_alloc_size);   
    BA_print_cur_node(0, obj, 0);
}

void BuddyAllocator_demo(void)
{
    size_t mem_size = 0;
    size_t minimum_size = 0;
    int command = 0;

    printf("Buddy Allocator demo\n");

    printf("type memory size:\n");
    scanf("%zu", &mem_size);

    printf("type memory minumum allocate size:\n");
    scanf("%zu", &minimum_size);

    Errors ret_code = OK;

    BuddyAllocator buddy_allocator;
    ret_code = BuddyAllocator_init(&buddy_allocator, mem_size, minimum_size);
    if (ret_code != OK) {
        printf("Error: %s\n", RetCode_get_str(ret_code));
        return;
    }

    MemMapDemo allocated_addresses_demo;
    MemMapDemo_init(&allocated_addresses_demo, &buddy_allocator, allocate_f, free_f);

    do {
        printf("--------\n");
        BuddyAllocator_print_info(&buddy_allocator);

        printf("type command:\n"
                "0 - quit\n"
                "1 - allocate\n"
                "2 - free\n"
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
                MemMapDemo_allocate_dialogue(&allocated_addresses_demo, true);
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

    } while(command != 0);

    MemMapDemo_deinit(&allocated_addresses_demo);
    BuddyAllocator_deinit(&buddy_allocator);
}