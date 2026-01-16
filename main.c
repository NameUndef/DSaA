#include <stdio.h>
#include <stdbool.h>
#include "ring_buffer_demo.h"
#include "portal_queue_demo.h"
#include "arena_allocator_demo.h"
#include "slab_allocator_demo.h"
#include "page_allocator_demo.h"
#include "pool_allocator_demo.h"

int main(int argc, char* argv[])
{
    int command = 0;
    bool quit = false;

    printf("Data Structures and Algorithms Demos\n");

    do {

        printf("type command:\n"
        "0 - quit\n"
        "1 - RingBuffer demo\n"
        "2 - PortalQueue demo\n"
        "3 - ArenaAllocator demo\n"
        "4 - PoolAllocator demo\n"
        "5 - PageAllocator demo\n"
        "6 - SlabAllocator demo\n");
        scanf("%d", &command);

        switch (command) {

            default: {
                printf("unknown command\n");
                break;
            }

            case 0: {
                quit = true;
                break;
            }

            case 1: {
                RingBuffer_demo();
                break;
            }

            case 2: {
                PortalQueue_demo();
                break;
            }

            case 3: {
                ArenaAllocator_demo();
                break;
            }

            case 4: {
                PoolAllocator_demo();
                break;
            }

            case 5: {
                PageAllocator_demo();
                break;
            }

            case 6: {
                SlabAllocator_demo();
                break;
            }
        }

    } while (!quit);
    return 0;
}
