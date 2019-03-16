#ifndef LEVEL_FOLDER_H_
#define LEVEL_FOLDER_H_

typedef struct LevelFolder LevelFolder;

LevelFolder *create_level_folder(const char *dirpath);
void destroy_level_folder(LevelFolder *level_folder);

const char **level_folder_filenames(const LevelFolder *level_folder);
const char **level_folder_titles(const LevelFolder *level_folder);
size_t level_folder_count(const LevelFolder *level_folder);

#endif  // LEVEL_FOLDER_H_
