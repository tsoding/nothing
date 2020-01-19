#ifndef EXTREMA_H_
#define EXTREMA_H_

#include <stdint.h>

#define MAX_INSTANCE(type)                      \
    static inline                               \
    type max_##type(type a, type b) {           \
        return a > b ? a : b;                   \
    }                                           \

MAX_INSTANCE(int64_t)
MAX_INSTANCE(size_t)
#define MAX(type, a, b) max_##type(a, b)

#define MIN_INSTANCE(type)                      \
    static inline                               \
    type min_##type(type a, type b) {           \
        return a < b ? a : b;                   \
    }                                           \

MIN_INSTANCE(int64_t)
MIN_INSTANCE(size_t)
#define MIN(type, a, b) min_##type(a, b)

#endif  // EXTREMA_H_
