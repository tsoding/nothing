#ifndef LEVEL_METADATA_H_
#define LEVEL_METADATA_H_

typedef struct LineStream LineStream;

#define VERSION "1"

#define METADATA_TITLE_MAX_SIZE 256
#define METADATA_VERSION_MAX_SIZE 256

typedef struct {
    char version[METADATA_VERSION_MAX_SIZE];
    char title[METADATA_TITLE_MAX_SIZE];
} LevelMetadata;

int metadata_load_from_file(LevelMetadata *metadata, const char *file);
int metadata_load_from_line_stream(LevelMetadata *metadata, LineStream *line_stream);

#endif  // LEVEL_METADATA_H_
