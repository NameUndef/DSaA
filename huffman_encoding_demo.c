#include "huffman_encoding_demo.h"

#include "huffman_encoding.h"
#include "utils.h"
#include <stdio.h>
#include <stdbool.h>

#define DEC_CAPACITY 1001
#define ENC_CAPACITY 1001

void HuffmanEncoding_demo()
{
    EncodeArray encodes = {.array = {{0, 0}}, .left_is_1 = 1};
    DecodeArray decodes;
    
    char decoded_str[DEC_CAPACITY] = {0};
    code_t encoded_data[ENC_CAPACITY] = {0};
    unsigned int enc_data_len = 0;
    unsigned int last_byte_len = 0;
    unsigned int dec_data_len = 0;

    while (true) {

        int command = 0;

        printf("type command:\n"
            "0 - quit\n"
            "1 - encode\n"
            "2 - decode\n");

        scanf("%d", &command);

        switch (command) {
            case 0: {
                break;
            }

            case 1: {
                printf("input str (max 1000):\n ");
                clear_stdin_buffer();
                fgets(decoded_str, DEC_CAPACITY - 1, stdin);

                huffman_encode(decoded_str, encoded_data, ENC_CAPACITY, &enc_data_len, &last_byte_len, &encodes, &decodes);
                break;
            }
            
            case 2: {

                huffman_decode(&decodes, encoded_data, enc_data_len, last_byte_len, decoded_str, DEC_CAPACITY, &dec_data_len);

                decoded_str[dec_data_len] = '\0';
                puts("\ndecoded string:\n");
                puts(decoded_str);
                break;
            }

            default: {
                printf("unknown command\n");
                break;
            }
        }

        if (!command) {
            break;
        }
    }
}
