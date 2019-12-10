#include <stdlib.h>
#include <stdio.h>

#include "system/stacktrace.h"
#include "system/nth_alloc.h"
#include "system/lt.h"
#include "system/line_stream.h"
#include "system/str.h"
#include "level_metadata.h"
#include "math/extrema.h"

int metadata_load_from_line_stream(LevelMetadata *metadata,
                                   LineStream *line_stream,
                                   const char *filepath)
{
    trace_assert(metadata);
    trace_assert(line_stream);

    memset(metadata->version, 0, METADATA_VERSION_MAX_SIZE);
    memset(metadata->title, 0, METADATA_TITLE_MAX_SIZE);
    memset(metadata->filepath, 0, METADATA_FILEPATH_MAX_SIZE);

    const char *line = line_stream_next(line_stream);
    if (line == NULL) return -1;

    memcpy(metadata->version,
           line,
           min_size_t(strlen(line), METADATA_VERSION_MAX_SIZE - 1));
    trim_endline(metadata->version);

    line = line_stream_next(line_stream);
    if (line == NULL) return -1;

    memcpy(metadata->title,
           line,
           min_size_t(strlen(line), METADATA_VERSION_MAX_SIZE - 1));
    trim_endline(metadata->title);

    memcpy(metadata->filepath,
           filepath,
           min_size_t(strlen(filepath), METADATA_FILEPATH_MAX_SIZE - 1));
    trim_endline(metadata->filepath);

    return 0;
}

int metadata_load_from_file(LevelMetadata *metadata, const char *filepath)
{
    trace_assert(metadata);
    trace_assert(filepath);

    LineStream *line_stream = create_line_stream(filepath, "r", 256);
    if (line_stream == NULL) return -1;

    int err = metadata_load_from_line_stream(metadata, line_stream, filepath);
    destroy_line_stream(line_stream);
    return err;
}
