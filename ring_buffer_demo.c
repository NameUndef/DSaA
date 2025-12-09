#include "ring_buffer.h"
#include <stdio.h>
#include "ret_code.h"

void RingBuffer_demo()
{
    size_t capacity = 0;
    Errors ret_code;
    bool quit = false;
    uint8_t value = 0;
    int command;

    printf("Ring buffer demo\n");
    printf("type RB capacity:\n");

    scanf("%u", &capacity);

    RingBuffer ring_buffer;
    ret_code = RingBuffer_init(&ring_buffer, capacity);

    if (ret_code != OK) {
        printf("error: %s\n", RetCode_get_str(ret_code));
        return;
    }

    do {
        printf("--------\n");
        printf("value: %d\n", value);
        printf("is full: %d\n", RingBuffer_is_full(&ring_buffer));

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
                ret_code = RingBuffer_push(&ring_buffer, value);
                if (ret_code != OK) {
                    printf("error: %s\n", RetCode_get_str(ret_code));
                } else {
                    printf("pushed\n");
                }
                break;
            }

            case 3: {
                uint8_t popped_value = 0;
                ret_code = RingBuffer_pop(&ring_buffer, &popped_value);
                if (ret_code != OK) {
                    printf("error: %s\n", RetCode_get_str(ret_code));
                } else {
                    printf("popped value: %d\n", popped_value);
                }
                break;
            }
        }

    } while (!quit);

    RingBuffer_deinit(&ring_buffer);
}
