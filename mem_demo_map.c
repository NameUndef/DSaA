#include "mem_demo_map.h"
#include "ret_code.h"
#include "utils.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

void MemMapDemo_init(MemMapDemo* dest, void* allocator_dest, allocate_func allocate, free_func free)
{
    if (!dest) {
        printf("Error: arguments are null\n");
        return;
    }

    dest->root = NULL;
    dest->allocate = allocate;
    dest->free = free;
    dest->allocator_dest = allocator_dest;
}

void MemMapDemo_deinit(MemMapDemo* dest)
{
    AllocatedAddress* current = dest->root;
    AllocatedAddress* next; 

    while (current) {
        if (current->count > 1) {
            free(current->ptr);
        }
        next = current->next;
        free(current);
        current = next;
    }
}

void MemMapDemo_allocate_dialogue(MemMapDemo* dest)
{
    AllocatedAddress* node = (void*) malloc(sizeof(AllocatedAddress));
    if (!node) {
        printf("error: %s\n", RetCode_get_str(ALLOC_ERROR));
        return;
    }
    printf("Allocate\n");
    printf("type name:\n");
    clear_stdin_buffer();
    if (fgets(node->name, sizeof(node->name), stdin) == NULL) {
        printf("error: %s\n", RetCode_get_str(BAD_ARGUMENT_OUT_OF_BOUNDS));
        return;
    }    
    size_t len = strlen(node->name);
    if (node->name[len - 1] == '\n') {
        node->name[len - 1] = '\0';
    }

    printf("type count:\n");
    scanf("%zu", &node->count);

    bool have_error = false;

    if (node->count == 1) {
        node->ptr = (void**) dest->allocate(dest->allocator_dest);
        if (!node->ptr) {
            printf("have failed allocate\n");
        }
    } else {
        node->ptr = (void**) malloc(node->count * sizeof(void*));
        if (!node->ptr) {
            printf("error: %s\n", RetCode_get_str(ALLOC_ERROR));
            free(node);
            return;
        }
        for (size_t i = 0; i < node->count; i++) {
            node->ptr[i] = dest->allocate(dest->allocator_dest);
            if (!node->ptr[i]) {
                printf("have failed allocate at %d\n", i);
            }
        }
    }

    node->prev = NULL;
    node->next = dest->root;
    if (node->next) {
        node->next->prev = node;
    }
    dest->root = node;
}

void MemMapDemo_free_dialogue(MemMapDemo* dest)
{
    printf("Deallocate\n");
    printf("type name:\n");
    char name[50];
    clear_stdin_buffer();
    if (fgets(name, sizeof(name), stdin) == NULL) {
        printf("error: %s\n", RetCode_get_str(BAD_ARGUMENT_OUT_OF_BOUNDS));
        return;
    }
    size_t len = strlen(name);
    if (name[len - 1] == '\n') {
        name[len - 1] = '\0';
    }

    bool find_name = false;
    AllocatedAddress* node = dest->root;
    while (node) {

        if (strcmp(node->name, name) == 0) {

            find_name = true;
            if (node->count == 1) {
                printf("free addr: %p\n", node->ptr);
                dest->free(dest->allocator_dest, (void*) node->ptr);
            } else {
                for (size_t i = 0; i < node->count; i++) {
                    printf("free addr: %p\n", node->ptr[i]);
                    dest->free(dest->allocator_dest, node->ptr[i]);
                }
                free(node->ptr);
            }
            break;
        }
        node = node->next;
    }

    if (!find_name) {
        printf("name not found\n");
        return;
    }

    printf("find %zu addresses\n", node->count);

    if (node->next) {
        node->next->prev = node->prev;
    }

    if (node->prev) {
        node->prev->next = node->next;
    } else {
        dest->root = node->next;
    }

    free(node);
}

void MemMapDemo_list(MemMapDemo* dest)
{
    AllocatedAddress *allocated_node = dest->root;

    while (allocated_node) {
        printf("node: %s, count: %zu\n", allocated_node->name, allocated_node->count);
        if (allocated_node->count == 1) {
            printf("    addr: %p\n", allocated_node->ptr);
        } else {
            for (size_t i = 0; i < allocated_node->count; i++) {
                printf("    addr: %p\n", allocated_node->ptr[i]);
            }
        }
        allocated_node = allocated_node->next;
    }
}