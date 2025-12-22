#include "ret_code.h" 

static char ret_code_str[RET_CODE_SIZE][RC_STR_SIZE] = {
    "OK",
    "UNDEFINED_ERROR",
    "ALLOC_ERROR",
    "BAD_ARGUMENT",
    "BAD_ARGUMENT_ZERO",
    "BAD_ARGUMENT_NULL_POINTER",
    "BAD_ARGUMENT_OUT_OF_BOUNDS",
    "FULL_BUFFER",
    "EMPTY_BUFFER"
};

const char * const RetCode_get_str(Errors code)
{
    return ret_code_str[code];
}