#include "ring_buffer_demo.h"
#include "portal_queue_demo.h"
#include "arena_allocator_demo.h"
#include "slab_allocator_demo.h"
#include "page_allocator_demo.h"
#include "pool_allocator_demo.h"
#include "huffman_encoding_demo.h"
#include "priorities_queue_demo.h"
#include "hash_map_demo.h"

#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>

typedef void(*demo_func)(void);

typedef struct {
    void (*func)(void);
    char name[100];
} Demo;

int main(int argc, char* argv[])
{
    Demo demos[] = {
        {RingBuffer_demo, "RingBuffer"},
        {PortalQueue_demo, "PortalQueue"},
        {ArenaAllocator_demo, "ArenaAllocator"},
        {PoolAllocator_demo, "PoolAllocator"},
        {PageAllocator_demo, "PageAllocator"},
        {SlabAllocator_demo, "SlabAllocator"},
        {HuffmanEncoding_demo, "HuffmanEncoding"},
        {PrioritiesQueue_demo, "PrioritiesQueue"},
        {HashMap_demo, "HashMap"}
    };

    size_t demos_size = sizeof (demos) / sizeof (demos[0]);

    int command = 0;
    
    printf("Data Structures and Algorithms Demos\n");

    do {

        printf("type command:\n");
        printf("0 - quit\n");
        for (size_t i = 0; i < demos_size; i++) {
            printf("%zu - %s demo\n", i + 1, demos[i].name);
        }

        scanf("%d", &command);

        if (!command) {
            break;
        } else if (command > demos_size || command <= -1) {
            printf("unknown command\n");
            continue;
        }

        demos[command - 1].func();

    } while (true);
    return 0;
}
