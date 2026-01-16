#ifndef INCLUDE_PAGE_MANAGE_H
#define INCLUDE_PAGE_MANAGE_H

#include <stdlib.h>

size_t get_page_size();
void* allocate_page();
void free_page(void* page);
void* allocate_page_sz(size_t page_size);
void free_page_sz(void* page, size_t page_size);

#endif  // INCLUDE_PAGE_MANAGE_H