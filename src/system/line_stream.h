#ifndef LINE_STREAM_H_
#define LINE_STREAM_H_

#include <stdlib.h>

typedef struct LineStream LineStream;

LineStream *create_line_stream(const char *filename,
                               const char *mode,
                               size_t capacity);
void destroy_line_stream(LineStream *line_stream);

const char *line_stream_next_chunk(LineStream *line_stream);
const char *line_stream_next(LineStream *line_stream);
char *line_stream_collect_n_lines(LineStream *line_stream, size_t n);
char *line_stream_collect_until_end(LineStream *line_stream);

#endif  // LINE_STREAM_H_
