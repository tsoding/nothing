#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "system/stacktrace.h"
#include "system/lt.h"
#include "system/lt_adapters.h"
#include "system/nth_alloc.h"
#include "system/log.h"
#include "system/line_stream.h"
#include "system/str.h"
#include "system/file.h"
#include "dynarray.h"
#include "game/level_metadata.h"

#include "./level_folder.h"

#define LEVEL_FOLDER_MAX_LENGTH 512

struct LevelFolder
{
    Lt *lt;
    Dynarray filenames;
    Dynarray titles;
};

LevelFolder *create_level_folder(const char *dirpath)
{
    trace_assert(dirpath);

    Lt *lt = create_lt();

    LevelFolder *level_folder = PUSH_LT(
        lt,
        nth_calloc(1, sizeof(LevelFolder)),
        free);
    if (level_folder == NULL) {
        RETURN_LT(lt, NULL);
    }
    level_folder->lt = lt;

    level_folder->filenames = create_dynarray(sizeof(const char*));
    level_folder->titles = create_dynarray(sizeof(const char*));

    char path[LEVEL_FOLDER_MAX_LENGTH];
    DIR *level_dir = PUSH_LT(lt, opendir(dirpath), closedir_lt);

    LevelMetadata level_metadata;
    for (struct dirent *d = readdir(level_dir);
         d != NULL;
         d = readdir(level_dir)) {
        if (*d->d_name == '.') {
            continue;
        }

        snprintf(path, LEVEL_FOLDER_MAX_LENGTH, "%s/%s", dirpath, d->d_name);
        const char *filepath = PUSH_LT(lt, string_duplicate(trim_endline(path), NULL), free);
        if (filepath == NULL) {
            RETURN_LT(lt, NULL);
        }

        if (metadata_load_from_file(&level_metadata, filepath) < 0) {
            RETURN_LT(lt, NULL);
        }

        const char *version = PUSH_LT(
            lt,
            string_duplicate(level_metadata.version, NULL),
            free);
        const char *title = PUSH_LT(
            lt,
            string_duplicate(level_metadata.title, NULL),
            free);

        if(strcmp(version, VERSION) == 0) {
            dynarray_push(&level_folder->titles, &title);
            dynarray_push(&level_folder->filenames, &filepath);
        } else {
            log_info(
                "Unsupported version for level [%s]: Expected `%s`, got `%s`\n",
                d->d_name,
                VERSION,
                version);
        }
    }

    closedir(RELEASE_LT(lt, level_dir));

    return level_folder;
}

void destroy_level_folder(LevelFolder *level_folder)
{
    trace_assert(level_folder);
    free(level_folder->filenames.data);
    free(level_folder->titles.data);
    RETURN_LT0(level_folder->lt);
}

const char **level_folder_filenames(const LevelFolder *level_folder)
{
    trace_assert(level_folder);
    return (const char **)level_folder->filenames.data;
}

const char **level_folder_titles(const LevelFolder *level_folder)
{
    trace_assert(level_folder);
    return (const char **)level_folder->titles.data;
}

size_t level_folder_count(const LevelFolder *level_folder)
{
    trace_assert(level_folder);
    return level_folder->filenames.count;
}
