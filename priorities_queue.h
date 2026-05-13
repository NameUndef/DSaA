#ifndef INCLUDE_PRIORITIES_QUEUE_H
#define INCLUDE_PRIORITIES_QUEUE_H

#include <stddef.h>

typedef struct _PrioritiesQueueNode {
    int weight;
    int data;
} PrioritiesQueueNode;

typedef struct {
    PrioritiesQueueNode *nodes;
    size_t nodes_capacity;
    size_t nodes_count;
} PrioritiesQueue;

int PrioritiesQueue_init(PrioritiesQueue *obj, size_t capacity);
int PrioritiesQueue_deinit(PrioritiesQueue *obj);
int PrioritiesQueue_push(PrioritiesQueue *obj, int weight, int data);
int PrioritiesQueue_pop(PrioritiesQueue *obj, int *weight, int *data);

#endif  // INCLUDE_PRIORITIES_QUEUE_H