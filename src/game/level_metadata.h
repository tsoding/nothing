#ifndef LEVEL_METADATA_H_
#define LEVEL_METADATA_H_

#include "config.h"

typedef struct LineStream LineStream;

typedef struct {
    char filepath[METADATA_FILEPATH_MAX_SIZE];
    char version[METADATA_VERSION_MAX_SIZE];
    char title[METADATA_TITLE_MAX_SIZE];
} LevelMetadata;

int metadata_load_from_file(LevelMetadata *metadata, const char *filepath);
int metadata_load_from_line_stream(LevelMetadata *metadata, LineStream *line_stream, const char *filepath);

#endif  // LEVEL_METADATA_H_
