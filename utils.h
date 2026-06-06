#ifndef INCLUDE_UTILS_H
#define INCLUDE_UTILS_H

#include <stdint.h>

void print_u8_bits(uint8_t value);
void print_u16_bits(uint16_t value);
void print_u32_bits(uint32_t value);
void print_u64_bits(uint64_t value);
void print_data(const uint8_t *data, unsigned int size, unsigned int last_byte_length);

uint32_t flip_u32_bits(uint32_t value, unsigned int size);
uint64_t flip_u64_bits(uint64_t value, unsigned int size);

void clear_stdin_buffer();

size_t get_0_bits_count_before_1(size_t value);
size_t ceil_to_power_of_2(size_t value);

#endif  // INCLUDE_UTILS_H
