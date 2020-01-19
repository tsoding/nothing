#include <stdio.h>

#include "file.h"
#include "lt_adapters.h"

void fclose_lt(void* file)
{
    fclose(file);
}

void closedir_lt(void *dir)
{
    closedir(dir);
}
