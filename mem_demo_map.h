#ifndef INCLUDE_MEM_DEMO_MAP_H
#define INCLUDE_MEM_DEMO_MAP_H

#include "ret_code.h"
#include <stdlib.h>

typedef struct _AllocatedAddress {

    char name[50];
    struct _AllocatedAddress* prev;
    struct _AllocatedAddress* next;
    void** ptr;
    size_t count;

} AllocatedAddress;

typedef void*   (*allocate_func) (void* dest);
typedef Errors  (*free_func)     (void* dest, void* address);

typedef struct {
    AllocatedAddress* root;
    void*             allocator_dest;
    allocate_func     allocate;
    free_func         free;
} MemMapDemo;

void MemMapDemo_init(MemMapDemo* dest, void* allocator_dest, allocate_func allocate, free_func free);
void MemMapDemo_deinit(MemMapDemo* dest);
void MemMapDemo_allocate_dialogue(MemMapDemo* dest);
void MemMapDemo_free_dialogue(MemMapDemo* dest);
void MemMapDemo_list(MemMapDemo* dest);

#endif  // INCLUDE_MEM_DEMO_MAP_H