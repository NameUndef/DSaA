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
    BAD_ARGUMENT_NOT_POWER_OF_2,
    ALREADY_FREE,
    FULL_BUFFER,
    EMPTY_BUFFER,
    NOT_FOUND,
    RET_CODE_SIZE
} Errors;

#define RC_STR_SIZE 30

const char * RetCode_get_str(Errors err);

#endif  // INCLUDE_RET_CODE_H
