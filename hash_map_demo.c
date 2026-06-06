#include "hash_map_demo.h"

#include "slab_allocator.h"
#include "page_allocator.h"
#include "hash_map.h"
#include "utils.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

static void HashMap_print_info(HashMap *obj)
{
    printf("map size: %zu, used: %zu\n", obj->map_size, obj->used);

    for (size_t i = 0; i < obj->map_size; i++) {

        BucketNode *node = obj->map[i];

        if (!node) {
            continue;
        }

        printf("%zu bucket:\n", i);

        size_t node_count = 0;

        while (node) {
            printf("    node: %s, data: %d\n", node->key, node->data);
            node = node->next;
            node_count++;
        }

        printf("node count: %zu\n", node_count);
    }
}

static void read_key(char key[KEY_SIZE])
{
    printf("type key: ");
    clear_stdin_buffer();
    fgets(key, KEY_SIZE, stdin);

    size_t len = strlen(key);
    if (key[len - 1] == '\n') {
        key[len - 1] = '\0';
    }
}

void HashMap_demo(void)
{
    size_t map_size = 0;
    printf("type map size in power of 2:\n");
    scanf("%zu", &map_size);

    PageAllocator page_allocator;
    SlabAllocator slab_allocator;
    HashMap hash_map;

    int ret_code = PageAllocator_init(&page_allocator, 0, 0);
    if (ret_code) {
        printf("error: %s\n", RetCode_get_str(ret_code));
        return;
    }

    HashMap_init(&hash_map, &slab_allocator, &page_allocator, map_size);

    while (true) {

        int command = 0;

        printf("--------\n"
                "0 - quit\n"
                "1 - set\n"
                "2 - get\n"
                "3 - remove\n"
                "4 - check\n"
                "5 - info\n");
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
                char key[KEY_SIZE] = {0};
                read_key(key);

                printf("type data:\n");
                int data = 0;
                scanf("%d", &data);

                ret_code = HashMap_set(&hash_map, key, data);
                if (ret_code) {
                    printf("error: %s\n", RetCode_get_str(ret_code));
                }
                break;
            }

            case 2: {
                char key[KEY_SIZE] = {0};
                read_key(key);
                int data = 0;
                ret_code = HashMap_get(&hash_map, key, &data);
                if (ret_code) {
                    printf("error: %s\n", RetCode_get_str(ret_code));
                    break;
                }
                printf("data: %d\n", data);
                break;
            }

            case 3: {
                char key[KEY_SIZE] = {0};
                read_key(key);
                bool result = false;
                ret_code = HashMap_remove(&hash_map, key);
                if (ret_code) {
                    printf("error: %s\n", RetCode_get_str(ret_code));
                    break;
                }

                printf("removed\n");
                break;
            }

            case 4: {
                char key[KEY_SIZE] = {0};
                read_key(key);
                bool result = false;
                ret_code = HashMap_is_have(&hash_map, key, &result);
                if (ret_code) {
                    printf("error: %s\n", RetCode_get_str(ret_code));
                    break;
                }

                if (result) {
                    printf("found!\n");
                } else {
                    printf("not found\n");
                }
                break;
            }

            case 5: {
                HashMap_print_info(&hash_map);
                break;
            }
        }

        if (!command) {
            break;
        }
    }

    HashMap_deinit(&hash_map);
    PageAllocator_deinit(&page_allocator);
}
