#ifndef EXTREMA_H_
#define EXTREMA_H_

#define MIN_TEMPLATE(T, NAME)                   \
    inline T NAME(T a, T b) {                   \
        return a < b ? a : b;                   \
    }

#define MAX_TEMPLATE(T, NAME)                   \
    inline T NAME(T a, T b) {                   \
        return a > b ? a : b;                   \
    }

MIN_TEMPLATE(float, min_float)
MAX_TEMPLATE(float, max_float)
MIN_TEMPLATE(int64_t, min_int64)
MAX_TEMPLATE(int64_t, max_int64)

#endif  // EXTREMA_H_
