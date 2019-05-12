#include <stdlib.h>
#include <stdio.h>

#include "system/stacktrace.h"
#include "system/nth_alloc.h"
#include "system/lt.h"
#include "system/line_stream.h"
#include "system/str.h"
#include "level_metadata.h"

struct LevelMetadata
{
    Lt lt;
    const char *title;
};

LevelMetadata *create_level_metadata_from_file(const char *filename)
{
    trace_assert(filename);

    LineStream *line_stream = create_line_stream(filename, "r", 256);
    if (line_stream == NULL) {
        return NULL;
    }

    LevelMetadata *level_metadata = create_level_metadata_from_line_stream(line_stream);
    destroy_line_stream(line_stream);

    return level_metadata;
}

LevelMetadata *create_level_metadata_from_line_stream(LineStream *line_stream)
{
    trace_assert(line_stream);

    Lt lt = {0};

    LevelMetadata *level_metadata = PUSH_LT(
        lt, nth_calloc(1, sizeof(LevelMetadata)), free);
    if (level_metadata == NULL) {
        RETURN_LT(lt, NULL);
    }
    level_metadata->lt = lt;


    level_metadata->title = PUSH_LT(
        lt,
        trim_endline(string_duplicate(line_stream_next(line_stream), NULL)),
        free);
    if (level_metadata->title == NULL) {
        RETURN_LT(lt, NULL);
    }

    return level_metadata;
}

void destroy_level_metadata(LevelMetadata *level_metadata)
{
    trace_assert(level_metadata);
    RETURN_LT0(level_metadata->lt);
}

const char *level_metadata_title(const LevelMetadata *level_metadata)
{
    trace_assert(level_metadata);
    return level_metadata->title;
}
