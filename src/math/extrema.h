#ifndef EXTREMA_H_
#define EXTREMA_H_

#include <stdint.h>

// WARNING! Any attempts to "generalize" or "improve" this translation
// unit will result in an instantly closed Pull Request without any
// further discussion.
static inline
int64_t max_int64(int64_t a, int64_t b)
{
    return a > b ? a : b;
}

static inline
size_t max_size_t(size_t a, size_t b)
{
    return a > b ? a : b;
}

static inline
size_t min_size_t(size_t a, size_t b)
{
    return a < b ? a : b;
}

#endif  // EXTREMA_H_
