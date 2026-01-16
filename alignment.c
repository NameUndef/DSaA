#include "alignment.h"

size_t get_aligned_value(size_t value, size_t alignment) 
{
    return (value + alignment - 1) & ~(alignment - 1);
}

size_t get_aligned_low_border_value(size_t value, size_t alignment)
{
    return value & ~(alignment - 1);
}

bool is_power_of_2(size_t value)
{
    return (value & ~(value - 1)) == 0;
}
