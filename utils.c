#include "utils.h"
#include <stdio.h>

void print_u8_bits(uint8_t value) 
{
    for (size_t i = 0; i < 8; i++) {
        putchar((value & ((uint8_t)1 << (7 - i)))? '1' : '0');
    }
}

void print_u16_bits(uint16_t value) 
{
    for (size_t i = 0; i < 16; i++) {
        putchar((value & ((uint16_t)1 << (15 - i)))? '1' : '0');
    }
}

void print_u32_bits(uint32_t value) 
{
    for (size_t i = 0; i < 32; i++) {
        putchar((value & (1U << (31 - i)))? '1' : '0');
    }
}

void print_u64_bits(uint64_t value) 
{
    for (size_t i = 0; i < 64; i++) {
        putchar((value & (1ULL << (63 - i)))? '1' : '0');
    }
}

void print_data(const uint8_t *data, unsigned int size, unsigned int last_byte_length)
{
    if (!size) {
        return;
    }

    for (size_t i = 0; i < size - 1; i++) {

        for (size_t j = 0; j < 8; j++) {
            putchar((data[i] & ((uint8_t)1 << j))? '1' : '0');
        }
    }

    for (size_t i = 0; i < last_byte_length; i++) {
        putchar((data[size - 1] & ((uint8_t)1 << i))? '1' : '0');
    }
}

uint32_t flip_u32_bits(uint32_t value, unsigned int size)
{
    if (size > sizeof (uint32_t) * 8) {
        size = sizeof (uint32_t) * 8;
    }

    uint32_t new_value = 0;

    for (unsigned int i = 0; i < size; i++) {
        new_value |= (!!(value & (1U << (size - 1 - i)))) << i;
    }

    return new_value;
}

uint64_t flip_u64_bits(uint64_t value, unsigned int size)
{
    if (size > sizeof (uint64_t) * 8) {
        size = sizeof (uint64_t) * 8;
    }

    uint64_t new_value = 0;

    for (unsigned int i = 0; i < size; i++) {
        new_value |= (!!(value & (1ULL << (size - 1 - i)))) << i;
    }

    return new_value;
}

void clear_stdin_buffer()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

size_t get_0_bits_count_before_1(size_t value)
{
    size_t count = 0;
    for (count = 0; count < sizeof(value) << 3ULL; count++) {
        if ((value >> count) & 1ULL) {
            break;
        }
    }

    return count;
}

size_t ceil_to_power_of_2(size_t value)
{
    value--;
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    value |= value >> 32;
    value++;

    return value;
}
