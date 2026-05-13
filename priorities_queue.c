#include "priorities_queue.h"
#include "ret_code.h"
#include <stdlib.h>
#include <stdbool.h>

#define DEF_CAPACITY 100

int PrioritiesQueue_init(PrioritiesQueue *obj, size_t capacity)
{
    if (obj == NULL) {
        return BAD_ARGUMENT_NULL_POINTER;
    }

    if (!capacity) {
        capacity = DEF_CAPACITY;
    }

    PrioritiesQueueNode *nodes = (PrioritiesQueueNode*) malloc(capacity * sizeof(PrioritiesQueueNode));
    if (nodes == NULL) {
        return ALLOC_ERROR;
    }

    obj->nodes = nodes;
    obj->nodes_capacity = capacity;
    obj->nodes_count = 0;

    return OK;
}

int PrioritiesQueue_deinit(PrioritiesQueue *obj)
{
    if (obj == NULL) {
        return BAD_ARGUMENT_NULL_POINTER;
    }

    free(obj->nodes);

    obj->nodes = NULL;
    obj->nodes_capacity = 0;
    obj->nodes_count = 0;
    
    return OK;
}

int PrioritiesQueue_push(PrioritiesQueue *obj, int weight, int data)
{
    if (obj == NULL) {
        return BAD_ARGUMENT_NULL_POINTER;
    }

    if (obj->nodes_count == obj->nodes_capacity) {
        return FULL_BUFFER;
    }
   
    obj->nodes[obj->nodes_count].weight = weight;
    obj->nodes[obj->nodes_count].data = data;

    size_t cur_node = obj->nodes_count;
    obj->nodes_count++;

    while (cur_node) {

        size_t parent_node = (cur_node - 1) / 2;

        if (obj->nodes[parent_node].weight >= obj->nodes[cur_node].weight) {
            break;
        }

        PrioritiesQueueNode tmp = obj->nodes[cur_node];
        obj->nodes[cur_node] = obj->nodes[parent_node];
        obj->nodes[parent_node] = tmp;

        cur_node = parent_node;
    }

    return OK;
}

int PrioritiesQueue_pop(PrioritiesQueue *obj, int *weight, int *data)
{
    if (obj == NULL) {
        return BAD_ARGUMENT_NULL_POINTER;
    }

    if (!obj->nodes_count) {
        return EMPTY_BUFFER;
    }

    PrioritiesQueueNode result = obj->nodes[0];
    obj->nodes_count--;
    obj->nodes[0] = obj->nodes[obj->nodes_count];

    size_t cur_node = 0;
    size_t left_child = 1;
    size_t right_child = 2;
    size_t next_node = cur_node;

    while (true) {

        int cur_node_weight = obj->nodes[cur_node].weight;

        if (left_child < obj->nodes_count) {

            int left_child_weight = obj->nodes[left_child].weight;

            if (right_child < obj->nodes_count  
                && obj->nodes[right_child].weight > left_child_weight 
                && obj->nodes[right_child].weight > cur_node_weight) {

                next_node = right_child;

            } else if (left_child_weight > cur_node_weight) {
                next_node = left_child;
            } else {
                break;
            }
            
        } else {
            break;
        }

        PrioritiesQueueNode tmp = obj->nodes[cur_node];
        obj->nodes[cur_node] = obj->nodes[next_node];
        obj->nodes[next_node] = tmp;

        left_child = 2 * next_node + 1;
        right_child = left_child + 1;
        cur_node = next_node;
    }

    if (weight) {
        *weight = result.weight;
    }

    if (data) {
        *data = result.data;
    }

    return OK;
}
