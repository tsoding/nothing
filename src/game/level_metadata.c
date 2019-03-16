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
    Lt *lt;
    const char *title;
};

LevelMetadata *create_level_metadata_from_file(const char *filename)
{
    trace_assert(filename);

    Lt *lt = create_lt();
    if (lt == NULL) {
        return NULL;
    }

    LevelMetadata *level_metadata = PUSH_LT(
        lt, nth_alloc(sizeof(LevelMetadata)), free);
    if (level_metadata == NULL) {
        RETURN_LT(lt, NULL);
    }
    level_metadata->lt = lt;

    LineStream *line_stream = PUSH_LT(
        lt,
        create_line_stream(filename, "w", 256),
        destroy_line_stream);
    if (line_stream == NULL) {
        RETURN_LT(lt, NULL);
    }

    level_metadata->title = PUSH_LT(
        lt,
        string_duplicate(line_stream_next(line_stream), NULL),
        free);
    if (level_metadata->title == NULL) {
        RETURN_LT(lt, NULL);
    }

    fclose(RELEASE_LT(lt, line_stream));

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
