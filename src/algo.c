#include <stdlib.h>
#include "./algo.h"

float rand_float(float max_value)
{
    return (float) rand() / ((float) RAND_MAX / max_value);
}
