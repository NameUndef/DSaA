#include "segmented_ring_buffer.h"
#include <stdio.h>
#include "ret_code.h"

void SegmentedRingBuffer_demo()
{
    size_t segment_capacity = 0;
    size_t init_capacity = 0;
    Errors ret_code;
    bool quit = false;
    uint8_t value = 0;
    int command;

    printf("Segmented Ring buffer demo\n");
    printf("type SRB segment capacity:\n");

    scanf("%u", &segment_capacity);

    printf("type SRB init capacity (0 is default):\n");

    scanf("%u", &init_capacity);

    SegmentedRingBuffer segmented_ring_buffer;
    ret_code = SegmentedRingBuffer_init(&segmented_ring_buffer, segment_capacity, init_capacity);

    if (ret_code != OK) {
        printf("error: %s\n", RetCode_get_str(ret_code));
        return;
    }

    do {
        printf("--------\n");
        printf("value: %d\n", value);
        printf("is empty: %d\n", SegmentedRingBuffer_is_empty(&segmented_ring_buffer));

        printf("type command number:\n"
            "0-quit\n"
            "1-value\n"
            "2-push\n"
            "3-pop\n");
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
                printf("type value:\n");
                scanf("%d", &value);
                break;
            }

            case 2: {
                ret_code = SegmentedRingBuffer_push(&segmented_ring_buffer, value);
                if (ret_code != OK) {
                    printf("error: %s\n", RetCode_get_str(ret_code));
                } else {
                    printf("pushed\n");
                }
                break;
            }

            case 3: {
                uint8_t popped_value = 0;
                ret_code = SegmentedRingBuffer_pop(&segmented_ring_buffer, &popped_value);
                if (ret_code != OK) {
                    printf("error: %s\n", RetCode_get_str(ret_code));
                } else {
                    printf("popped value: %d\n", popped_value);
                }
                break;
            }
        }

    } while (!quit);

    SegmentedRingBuffer_deinit(&segmented_ring_buffer);
}
