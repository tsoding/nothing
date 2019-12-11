#ifndef LEVEL_FOLDER_H_
#define LEVEL_FOLDER_H_

#include "level_metadata.h"

typedef struct {
    Dynarray metadatas;
} LevelFolder;

static inline
LevelFolder create_level_folder(void)
{
    LevelFolder result = {
        .metadatas = create_dynarray(sizeof(LevelMetadata)),
    };
    return result;
}

static inline
void destroy_level_folder(LevelFolder level_folder)
{
    free(level_folder.metadatas.data);
}

void level_folder_read(const char *dirpath, LevelFolder *folder);

#endif  // LEVEL_FOLDER_H_
