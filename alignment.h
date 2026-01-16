#ifndef INCLUDE_ALIGNMENT_H
#define INCLUDE_ALIGNMENT_H

#include <stdlib.h>
#include <stdbool.h>

size_t get_aligned_value(size_t value, size_t alignment);

size_t get_aligned_low_border_value(size_t value, size_t alignment);

bool is_power_of_2(size_t value);

#endif  // INCLUDE_ALIGNMENT_H