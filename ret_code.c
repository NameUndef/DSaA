#include "ret_code.h" 

static char ret_code_str[RET_CODE_SIZE][RC_STR_SIZE] = {
   "OK",
   "UNDEFINED_ERROR",
   "ALLOC_ERROR",
   "BAD_ARGUMENT",
   "BAD_ARGUMENT_ZERO",
   "BAD_ARGUMENT_NULL_POINTER",
   "BAD_ARGUMENT_OUT_OF_BOUNDS",
   "BAD_ARGUMENT_NOT_POWER_OF_2",
   "ALREADY_FREE",
   "FULL_BUFFER",
   "EMPTY_BUFFER",
   "NOT_FOUND"
};

const char * RetCode_get_str(Errors code)
{
    return ret_code_str[code];
}