#include <stdlib.h>

#include "math/rand.h"

float rand_float(float max_value)
{
    return (float) rand() / ((float) RAND_MAX / max_value);
}

float rand_float_range(float lower, float upper)
{
    return rand_float(upper - lower) + lower;
}
