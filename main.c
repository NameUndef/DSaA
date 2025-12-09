#include <stdio.h>
#include <stdbool.h>
#include "ring_buffer_demo.h"

int main(int argc, char* argv[])
{
    int command = 0;
    bool quit = false;

    printf("Data Structures and Algorithms Demos\n");

    do {

        printf("type command:\n"
        "0 - quit\n"
        "1 - RingBuffer demo\n");
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
        }

    } while (!quit);
    return 0;
}