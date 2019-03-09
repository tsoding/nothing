#include <stdlib.h>

#include "system/stacktrace.h"

#include "./level_folder.h"

LevelFolder *create_level_folder(const char *dirpath)
{
    trace_assert(dirpath);
    return NULL;
}

void destroy_level_folder(LevelFolder *level_folder)
{
    trace_assert(level_folder);
}

const char **level_folder_files(const LevelFolder *level_folder)
{
    trace_assert(level_folder);
    return 0;
}

size_t level_folder_count(const LevelFolder *level_folder)
{
    trace_assert(level_folder);
    return 0;
}
