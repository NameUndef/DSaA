#include <stddef.h>

#if (defined(__GNUC__) || defined(__clang__)) && __STDC_VERSION__ < 202311L

#define container_of(ptr, type, member) ({                      \
    const __typeof__(((type *)0)->member) * __mptr = (ptr);     \
    (type *)((char *)__mptr - offsetof(type, member)); })

#elif __STDC_VERSION__ >= 202311L

#define container_of(ptr, type, member) ({                      \
    const typeof(((type *)0)->member) * __mptr = (ptr);         \
    (type *)((char *)__mptr - offsetof(type, member)); })

#else

#define container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))

#endif
