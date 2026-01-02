#include "portal_queue.h"
#include <stdio.h>

static void PortalQueue_print_info(PortalQueue* dest)
{
    printf("Segment capacity: %d\nis full: %d\nis empty: %d\nhave portal: %d\n", 
        dest->segment_capacity, PortalQueue_is_full(dest), PortalQueue_is_empty(dest), dest->segment_portal_src != NULL);
    Segment* segment_end, *segment;
    segment = segment_end = dest->used_segments_ring;

    do {
        for (size_t i = 0; i < dest->segment_capacity; i++) {
            uint8_t value = segment->data[i];
            printf("%03d|", value);
        }

        segment = segment->next;

        if (segment != segment_end) {
            printf("-");
        } else {
            break;
        }

    } while (true);
    printf("\n");

    segment = segment_end = dest->used_segments_ring;

    char headers[5];
    headers[4] = '\0';

    do {
        for (size_t i = 0; i < dest->segment_capacity; i++) {

            headers[1] = (segment == dest->cur_segment_head && i == dest->head)? 'h' : ' ';
            headers[2] = (segment == dest->cur_segment_tail && i == dest->tail)? 't' : ' ';
            headers[3] = (segment == dest->segment_portal_src && i == dest->window_size)? 's' : ' ';
            headers[0] = (segment == dest->segment_portal_dest && i == 0)? 'd' : ' ';

            printf("%s", headers);
        }

        segment = segment->next;

        if (segment != segment_end) {
            printf(" ");
        } else {
            break;
        }

    } while (true);
    printf("\n");
}

void PortalQueue_demo()
{
    size_t segment_capacity = 0;
    size_t capacity = 0;
    Errors ret_code;
    bool quit = false;
    uint8_t value = 0;
    bool free_empty_segments = false;
    bool auto_increment = false;
    bool reset_popped_values = false;
    int command;

    unsigned int tmp = 0;

    printf("Portal Queue demo\n");

    printf("type segment capacity:\n");
    scanf("%zu", &segment_capacity);

    printf("type init capacity:\n");
    scanf("%zu", &capacity);

    printf("free empty segments: 0 - no, 1 - yes\n");
    tmp = 0;
    scanf("%u", &tmp);
    free_empty_segments = !!tmp;

    printf("auto increment value: 0 - no, 1 - yes\n");
    tmp = 0;
    scanf("%u", &tmp);
    auto_increment = !!tmp;

    printf("reset popped values: 0 - no, 1 - yes\n");
    tmp = 0;
    scanf("%u", &tmp);
    reset_popped_values = !!tmp;

    PortalQueue portal_queue;
    ret_code = PortalQueue_init(&portal_queue, segment_capacity, capacity, free_empty_segments, reset_popped_values);
    if (ret_code != OK) {
        printf("error: %s\n", RetCode_get_str(ret_code));
        return;
    }

    do {
        printf("--------\n");
        printf("value: %d\n", value);
        printf("free empty segments: %d\n", free_empty_segments);
        printf("auto increment: %d\n", auto_increment);
        printf("reset popped values: %d\n", reset_popped_values);
        PortalQueue_print_info(&portal_queue);

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
                ret_code = PortalQueue_push(&portal_queue, value);
                if (ret_code != OK) {
                    printf("error: %s\n", RetCode_get_str(ret_code));
                } else {
                    printf("pushed\n");
                }

                if (auto_increment) {
                    value++;
                }

                break;
            }

            case 3: {
                uint8_t popped_value = 0;
                ret_code = PortalQueue_pop(&portal_queue, &popped_value);
                if (ret_code != OK) {
                    printf("error: %s\n", RetCode_get_str(ret_code));
                } else {
                    printf("popped value: %d\n", popped_value);
                }
                break;
            }
        }

    } while (!quit);

    PortalQueue_deinit(&portal_queue);
}
