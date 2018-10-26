#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "error.h"
#include "line_stream.h"
#include "lt.h"
#include "lt/lt_adapters.h"

struct LineStream
{
    Lt *lt;
    FILE *stream;
    char *buffer;
    size_t capacity;
};

LineStream *create_line_stream(const char *filename,
                               const char *mode,
                               size_t capacity)
{
    assert(filename);
    assert(mode);

    Lt *lt = create_lt();
    if (lt == NULL) {
        return NULL;
    }

    LineStream *line_stream = PUSH_LT(
        lt,
        malloc(sizeof(LineStream)),
        free);
    if (line_stream == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }
    line_stream->lt = lt;

    line_stream->stream = PUSH_LT(
        lt,
        fopen(filename, mode),
        fclose_lt);
    if (line_stream->stream == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    line_stream->buffer = PUSH_LT(
        lt,
        malloc(sizeof(char) * capacity),
        free);
    if (line_stream->buffer == NULL) {
        throw_error(ERROR_TYPE_LIBC);
        RETURN_LT(lt, NULL);
    }

    line_stream->capacity = capacity;

    return line_stream;
}

void destroy_line_stream(LineStream *line_stream)
{
    assert(line_stream);

    RETURN_LT0(line_stream->lt);
}

const char *line_stream_next(LineStream *line_stream)
{
    assert(line_stream);
    return fgets(line_stream->buffer,
                 (int) line_stream->capacity,
                 line_stream->stream);
}
