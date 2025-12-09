#ifndef INCLUDE_RET_CODE_H
#define INCLUDE_RET_CODE_H

typedef enum {
    OK,
    UNDEFINED_ERROR,
    ALLOC_ERROR,
    BAD_ARGUMENT,
    BAD_ARGUMENT_ZERO,
    BAD_ARGUMENT_NULL_POINTER,
    BAD_ARGUMENT_OUT_OF_BOUNDS,
    FULL_BUFFER,
    EMPTY_BUFFER,
    RET_CODE_SIZE
} Errors;

#define RC_STR_SIZE 30

const char * const RetCode_get_str(Errors err);

#endif  // INCLUDE_RET_CODE_H
