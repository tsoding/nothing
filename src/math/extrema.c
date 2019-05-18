#include <stdlib.h>
#include "./extrema.h"

int64_t max_int64(int64_t a, int64_t b)
{
    return a > b ? a : b;
}

size_t max_size_t(size_t a, size_t b)
{
    return a > b ? a : b;
}
