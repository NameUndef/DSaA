#include "huffman_encoding.h"

#include <stdio.h>
#include <stdbool.h>
#include "utils.h"

#define CODE_SIZE 8
#define STACK_SIZE 32

typedef struct {
    unsigned int array[MAX_CODE][2];
    unsigned int size;
} FreqArray;

typedef struct _DirTreeNode {
    struct _DirTreeNode *left;
    struct _DirTreeNode *right;
    unsigned int weight;
    char data; 
} DirTreeNode;

typedef struct _ListNode {
    struct _ListNode *prev;
    struct _ListNode *next;
    DirTreeNode *node;
} ListNode;

typedef struct { 
    DirTreeNode nodes[MAX_CODE + MAX_CODE - 1];
    unsigned int size;
} DirTree;

typedef struct {
    ListNode nodes[MAX_CODE];
    unsigned int size;
    ListNode *first;
    ListNode *last;
} List;

static unsigned int fill_freq_array(const char* str, FreqArray *data)
{
    unsigned int map[MAX_CODE] = {0};
    unsigned int cur_size = 0;
    unsigned int str_size = 0;
    
    for (; str[str_size]; str_size++) {
        
        unsigned int idx = (unsigned int) str[str_size];
        
        if (!map[idx]) {
            data->array[cur_size][0] = idx;
            data->array[cur_size][1] = 0;
            map[idx] = cur_size + 1;
            cur_size++;
        }
        
        data->array[map[idx] - 1][1]++;
    }
    
    data->size = cur_size;

    return str_size;
}

static void full_node_list(List *list, DirTree *tree, FreqArray *freq_array)
{
    tree->size = 0;
    list->size = 0;
    list->first = NULL;
    list->last = NULL;
    
    for (unsigned int i = 0; i < freq_array->size; i++) {
        
        ListNode* prev = NULL;
        ListNode* next = list->first;
        
        while (next && next->node->weight > freq_array->array[i][1]) {
            prev = next;
            next = next->next;
        }
        
        DirTreeNode *dt_node = &tree->nodes[tree->size];
        tree->size++;
        
        ListNode *list_node = &list->nodes[list->size];
        list->size++;
        
        dt_node->left = NULL;
        dt_node->right = NULL;
        dt_node->weight = freq_array->array[i][1];
        dt_node->data = (char) freq_array->array[i][0];
        
        list_node->node = dt_node;
        list_node->next = next;
        list_node->prev = prev;
        
        if (next) {            
            next->prev = list_node;            
        } else {
            list->last = list_node;
        }
        
        if (prev) {
            prev->next = list_node;
        } else {
            list->first = list_node;
        }
    }
}

static void build_dir_tree(List* list, DirTree *tree)
{   
   while (list->first != list->last) {
       
       ListNode* next = list->last;
       ListNode* prev = next->prev;
       
       list->last = prev->prev;
       
       if (prev->prev) {
           prev->prev->next = NULL;
       } else {
           list->first = NULL;
       }
       
       DirTreeNode *dt_node = &tree->nodes[tree->size];
       tree->size++;
       
       dt_node->left = prev->node;
       dt_node->right = next->node;
       dt_node->weight = prev->node->weight + next->node->weight;
       dt_node->data = '-';
       
       ListNode *target = prev;
       
       target->node = dt_node;
       
       
       prev = NULL;
       next = list->first;
       
       while (next && next->node->weight > dt_node->weight) {
           prev = next;
           next = next->next;
       }
       
       target->next = next;
       target->prev = prev;
          
       if (next) {            
           next->prev = target;            
       } else {
           list->last = target;
       }
        
       if (prev) {
           prev->next = target;
       } else {
           list->first = target;
       }
       
   }
}

static void print_dt_node(int depth, DirTreeNode *node)
{
    for (int i = 0; i < depth; i++) {
        putchar(' ');
    }
    
    if (node->data == '\n') {
        printf("\\n: %u\n", node->weight);
    } else {
        printf("%c: %u\n", node->data, node->weight);
    }
    if (node->left) {
        print_dt_node(depth + 4, node->left);
    }
    
    if (node->right) {
        print_dt_node(depth + 4, node->right);
    }
    
}

static void fill_codes_array(DirTreeNode *root, EncodeArray *encodes)
{
    if (!root->left) {
        encodes->count = 1;
        encodes->array[(unsigned int) root->data][1] = 1;
        encodes->array[(unsigned int) root->data][0] = 0;
        return;
    }

    encodes->count = 0;
    encodes->max_code_lengh = 0;

    DirTreeNode* stack[STACK_SIZE];
    unsigned int stack_size = 0;
    unsigned int code = 0;
    
    stack[stack_size] = root;
    stack_size++;

    DirTreeNode* prev = NULL, *next = NULL;
    
    while (stack_size) {

        unsigned int stack_pos = stack_size - 1;
        
        prev = next;
        next = stack[stack_pos];
        
        if (prev) {
            if (prev->left == next) {

                if (encodes->left_is_1) {
                    code |= 1 << stack_pos;
                } else {
                    code &= ~(1 << stack_pos);
                }

            } else if (prev->right == next) {

                if (encodes->left_is_1) {
                    code &= ~(1 << stack_pos);
                } else {
                    code |= 1 << stack_pos;
                }

            } else if (prev == next->left) {
                stack[stack_size] = next->right;
                stack_size++;
                continue;

            } else if (prev == next->right) {
                stack_size--;
                continue;
            }
        }

        if (next->left) {
            stack[stack_size] = next->left;
            stack_size++;
        } else {
            encodes->array[(unsigned int) next->data][1] = stack_pos;
            encodes->array[(unsigned int) next->data][0] = code >> 1;
            encodes->count++;
            stack_size--;

            encodes->max_code_lengh = encodes->max_code_lengh < (char) stack_pos? (char) stack_pos : encodes->max_code_lengh;
        }
    }
}

static int encode_data(
    const char *data, 
    unsigned int data_size, 
    const EncodeArray *array, 
    code_t *encoded_data, 
    unsigned int encoded_data_size,
    unsigned int *result_length,
    unsigned int *last_byte_length)
{
    if (encoded_data_size < data_size) {
        return -1;
    }

    unsigned int cur_encode_buffer_pos = 0;
    unsigned int cur_encode_pos = 0;
    unsigned int low_half = 0;
    unsigned high_half = 0;

    for (unsigned int i = 0; i < data_size; i++) {

        code_t code = array->array[(unsigned int) data[i]][0];
        char code_size = array->array[(unsigned int) data[i]][1];

        if (!code_size) {
            return -2;
        }

        if (CODE_SIZE - (char) cur_encode_buffer_pos < code_size) {

            low_half = CODE_SIZE - cur_encode_buffer_pos;
            high_half = code_size - low_half;

            encoded_data[cur_encode_pos] |= code << cur_encode_buffer_pos;

            cur_encode_pos++;
            encoded_data[cur_encode_pos] |= code >> low_half;

            cur_encode_buffer_pos = high_half;

        } else {
            encoded_data[cur_encode_pos] |= code << cur_encode_buffer_pos;
            cur_encode_buffer_pos += (unsigned int) code_size;
        } 

        if (data[i] == '\n') {
            printf("\n+\\n: ");
        } else {
            printf("\n+%c: ", data[i]);
        }
        print_data(encoded_data, cur_encode_pos + 1, cur_encode_buffer_pos);
    }

    *result_length = cur_encode_pos + 1;
    *last_byte_length = cur_encode_buffer_pos;

    return 0;
}

static void convert_to_decode(const EncodeArray *encodes, DecodeArray *decodes)
{
    char max_code_length = encodes->max_code_lengh;
    char code_reminder = CODE_SIZE - max_code_length;

    for (unsigned int i = 0; i < MAX_CODE; i++) {

        char code_size = encodes->array[i][1];
        if (!code_size) {
            continue;
        }

        code_t code_last_suffix = 0xFF >> (code_size + code_reminder);
        code_t code_base = encodes->array[i][0];

        for (code_t cur_suffix = 0; cur_suffix <= code_last_suffix; cur_suffix++) {

            code_t code = code_base | (cur_suffix << code_size);

            decodes->array[code][0] = i;
            decodes->array[code][1] = code_size;
        }
    }

    decodes->max_code_length = max_code_length;
}

void huffman_encode(
    const char *decoded_str, 
    code_t *encoded_data, 
    unsigned int enc_capacity, 
    unsigned int *new_enc_data_len, 
    unsigned int *last_byte_len,
    EncodeArray *encodes,
    DecodeArray *decodes)
{   
    FreqArray freq_data;
    
    unsigned int str_size = fill_freq_array(decoded_str, &freq_data);

    if (!str_size) {
        printf("string is empty\n");
        return;
    }

    printf("freq array:\n");
    for (unsigned int i = 0; i < freq_data.size; i++) {
        printf("%c: %u\n", 
        (char) freq_data.array[i][0],
        freq_data.array[i][1]);
        
    }
    
    DirTree tree;
    List list;
    
    full_node_list(&list, &tree, &freq_data);

    printf("\ndir tree/list:\n");
    for (ListNode *node = list.first; node; node = node->next) {
        DirTreeNode *tree_node = node->node;
        
        printf("%c: %u\n", tree_node->data, tree_node->weight);
    }
    
    build_dir_tree(&list, &tree);
    
    printf("\nbuilt dir tree:\n");
    print_dt_node(0, list.first->node);

    for (unsigned int i = 0; i < MAX_CODE; i++) {
        encodes->array[i][0] = 0;
        encodes->array[i][1] = 0;
        decodes->array[i][0] = 0;
        decodes->array[i][1] = 0;
    }
    encodes->count = 0;

    fill_codes_array(list.first->node, encodes);
    
    printf("\nencodes array, (max code length %u):", encodes->max_code_lengh);
    for (unsigned int i = 0; i < MAX_CODE; i++) {

        if (encodes->array[i][1]) {
            printf("\n%c: %u ", (char) i, encodes->array[i][1]);
            print_u32_bits(encodes->array[i][0]);
        }
    }

    unsigned int result_length = 0;
    unsigned int last_byte_length = 0;

    int res = encode_data(decoded_str, str_size, encodes, encoded_data, enc_capacity, &result_length, &last_byte_length);

    if (res) {
        printf("error: %d\n", res);
        return;
    }

    printf("\nlength: %u bits (%u bytes, last byte used length: %u)\n\"", (result_length - 1) * 8 + last_byte_length, result_length, last_byte_length);
    print_data(encoded_data, result_length, last_byte_length);
    putchar('\"');

    *new_enc_data_len = result_length;
    *last_byte_len = last_byte_length;

    convert_to_decode(encodes, decodes);

    printf("\n\ndecode array:");
    for (unsigned int i = 0; i < MAX_CODE; i++) {
        if (!decodes->array[i][1]) {
            continue;
        }

        printf("\n%c, %u: ", decodes->array[i][0], decodes->array[i][1]);
        print_u8_bits((char)i);
    }

    puts("\n\n");
}

static int decode(
    code_t first_encode, 
    code_t second_encode, 
    code_t code_mask, 
    char *cur_bit_pos, 
    char *decoded, 
    bool *next_encode,
    const DecodeArray *decodes)
{
        char next_cur_bit_pos = *cur_bit_pos;

        unsigned short full_encode = first_encode | ((unsigned short)second_encode << CODE_SIZE);
        code_t code = ((code_t)(full_encode >> next_cur_bit_pos)) & code_mask;


        if (!decodes->array[code][1]) {
            return -1;
        }

        *decoded = decodes->array[code][0];

        print_u8_bits(code);
        printf(" -> %c\n", *decoded);

        next_cur_bit_pos += decodes->array[code][1];
        *next_encode = next_cur_bit_pos >= CODE_SIZE;
        next_cur_bit_pos = *next_encode? next_cur_bit_pos - CODE_SIZE : next_cur_bit_pos;
        *cur_bit_pos = next_cur_bit_pos;

        return 0;
}

int huffman_decode(
    const DecodeArray *decodes, 
    const code_t *encoded_data, 
    unsigned int encoded_data_length, 
    unsigned int encoded_data_last_byte_length,
    char *decoded_data,
    unsigned int decoded_data_capacity,
    unsigned int *decoded_size)
{
    char cur_bit_pos = 0;

    code_t code_mask = 0xFF >> (CODE_SIZE - decodes->max_code_length);

    unsigned int j = 0;
    code_t first_encode = encoded_data[0], 
          second_encode = encoded_data[1];
    bool next_encode = false;

    *decoded_size = 0;

    for (unsigned int i = 1; i < encoded_data_length && j < decoded_data_capacity - 1;) {

        // print_u8_bits(first_encode);
        // putchar('\n');
        // print_u8_bits(second_encode);
        // putchar('\n');

        if (decode(first_encode, second_encode, code_mask, &cur_bit_pos, &decoded_data[j], &next_encode, decodes)) {
            return -1;
        }

        if (next_encode) {
            i++;
            first_encode = second_encode;
            second_encode = encoded_data[i];
        }

        j++;
    }

    for (; j < decoded_data_capacity - 1 && cur_bit_pos != (char) encoded_data_last_byte_length; j++) {

        if (decode(encoded_data[encoded_data_length - 1], 0, code_mask, &cur_bit_pos, &decoded_data[j], &next_encode, decodes)) {
            return -1;
        }

    }

    *decoded_size = j;
    return 0;
}
