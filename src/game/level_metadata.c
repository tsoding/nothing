#include <stdlib.h>

#include "system/stacktrace.h"
#include "level_metadata.h"

LevelMetadata *create_level_metadata_from_file(const char *filename)
{
    trace_assert(filename);
    return NULL;
}

void destroy_level_metadata(LevelMetadata *level_metadata)
{
    trace_assert(level_metadata);
}

const char *level_metadata_title(const LevelMetadata *level_metadata)
{
    trace_assert(level_metadata);
    return NULL;
}
