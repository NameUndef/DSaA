#ifndef INCLUDE_HUFFMAN_ENCODING_H
#define INCLUDE_HUFFMAN_ENCODING_H

#define MAX_CODE 256

typedef unsigned char code_t;

typedef struct {
    code_t array[MAX_CODE][2];
    int left_is_1;
    unsigned int count;
    char max_code_lengh;
} EncodeArray;

typedef struct {
    char array[256][2];
    char max_code_length;
} DecodeArray;

void huffman_encode(
    const char *decoded_str, 
    code_t *encoded_data, 
    unsigned int enc_capacity, 
    unsigned int *new_enc_data_length, 
    unsigned int *last_byte_length,
    EncodeArray *encodes,
    DecodeArray *decodes);

int huffman_decode(
    const DecodeArray *decodes, 
    const code_t *encoded_data, 
    unsigned int encoded_data_length, 
    unsigned int encoded_data_last_byte_length,
    char *decoded_data,
    unsigned int decoded_data_capacity,
    unsigned int *decoded_size);

#endif  // INCLUDE_HUFFMAN_ENCODING_H
