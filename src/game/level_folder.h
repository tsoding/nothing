#ifndef LEVEL_FOLDER_H_
#define LEVEL_FOLDER_H_

typedef struct {
    Dynarray filepaths;
} LevelFolder;

static inline
LevelFolder create_level_folder(void)
{
    LevelFolder result = {
        .filepaths = create_dynarray(METADATA_FILEPATH_MAX_SIZE),
    };
    return result;
}

static inline
void destroy_level_folder(LevelFolder level_folder)
{
    free(level_folder.filepaths.data);
}

void level_folder_read(const char *dirpath, LevelFolder *folder);

#endif  // LEVEL_FOLDER_H_
