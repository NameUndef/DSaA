#include "priorities_queue_demo.h"

#include "priorities_queue.h"
#include "ret_code.h"
#include <stdio.h>
#include <stdbool.h>

static void print_node(int depth, PrioritiesQueue *obj, size_t node_idx)
{
    for (int i = 0; i < depth; i++) {
        putchar(' ');
    }

    printf("node weight: %d, data: %d\n", obj->nodes[node_idx].weight, obj->nodes[node_idx].data);

    size_t left = 2 * node_idx + 1;
    size_t right = left + 1;

    if (left < obj->nodes_count) {
        print_node(depth + 4, obj, left);
    } else {
        return;
    }

    if (right < obj->nodes_count) {
        print_node(depth + 4, obj, right);
    }
}

static void PrioritiesQueue_print_info(PrioritiesQueue *obj)
{
    printf("count: %zu", obj->nodes_count);
}

void PrioritiesQueue_demo(void)
{
    size_t capacity = 0;

    printf("Type priorities queue capacity: ");
    scanf("%zu", &capacity);

    PrioritiesQueue queue;
    int data = 0;

    PrioritiesQueue_init(&queue, capacity);

    while (true) {

        if (queue.nodes_count) {
            print_node(0, &queue, 0);
        }

        int command = 0;

        printf("type command:\n"
            "0 - quit\n"
            "1 - push\n"
            "2 - pop\n");

        scanf("%d", &command);

        switch (command) {

            case 0: {
                break;
            }

            case 1: {
                printf("type weight:\n");
                int weight = 0;

                scanf("%d", &weight);

                int ret_code = PrioritiesQueue_push(&queue, weight, data);
                if (ret_code) {
                    printf("error: %s\n", RetCode_get_str(ret_code));
                    break;
                }

                printf("push, weight: %d, data: %d\n", weight, data);

                data++;
                if (data > 1000) {
                    data = 0;
                }

                break;
            }

            case 2: {
                int res_weight = 0, res_data = 0;
                int ret_code = PrioritiesQueue_pop(&queue, &res_weight, &res_data);

                if (ret_code) {
                    printf("error: %s\n", RetCode_get_str(ret_code));
                    break;
                }

                printf("pop, weight: %d, data: %d\n", res_weight, res_data);
                break;
            }

            default: {
                printf("unknown command\n");
                break;
            }
        }

        if (command == 0) {
            break;
        }
    }

    PrioritiesQueue_deinit(&queue);
}