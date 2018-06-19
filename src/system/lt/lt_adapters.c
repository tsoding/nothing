#include "lt_adapters.h"
#include <stdio.h>

void fclose_lt(void* file)
{
    fclose(file);
}
