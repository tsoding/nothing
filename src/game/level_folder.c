#include <stdio.h>
#include <stdlib.h>

#include "system/stacktrace.h"
#include "system/lt.h"
#include "system/lt_adapters.h"
#include "system/nth_alloc.h"
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
    Dynarray *filenames;
    Dynarray *titles;
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

    level_folder->filenames = PUSH_LT(
        lt,
        create_dynarray(sizeof(const char*)),
        destroy_dynarray);
    if (level_folder->filenames == NULL) {
        RETURN_LT(lt, NULL);
    }

    level_folder->titles = PUSH_LT(
        lt,
        create_dynarray(sizeof(const char*)),
        destroy_dynarray);
    if (level_folder->titles == NULL) {
        RETURN_LT(lt, NULL);
    }

    char path[LEVEL_FOLDER_MAX_LENGTH];
    // TODO(#920): Level Picker is probably broken on Windows
    //   We have never tested our dirent implementation on Windows. So
    //   it's probably broken. We need to check it.
    DIR *level_dir = PUSH_LT(lt, opendir(dirpath), closedir_lt);

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

        LevelMetadata *level_metadata = create_level_metadata_from_file(filepath);
        if (level_metadata == NULL) {
            RETURN_LT(lt, NULL);
        }
        const char *title = PUSH_LT(
            lt,
            string_duplicate(level_metadata_title(level_metadata), NULL),
            free);
        destroy_level_metadata(level_metadata);

        dynarray_push(level_folder->titles, &title);
        dynarray_push(level_folder->filenames, &filepath);
    }

    closedir(RELEASE_LT(lt, level_dir));

    return level_folder;
}

void destroy_level_folder(LevelFolder *level_folder)
{
    trace_assert(level_folder);
    RETURN_LT0(level_folder->lt);
}

const char **level_folder_filenames(const LevelFolder *level_folder)
{
    trace_assert(level_folder);
    return dynarray_data(level_folder->filenames);
}

const char **level_folder_titles(const LevelFolder *level_folder)
{
    trace_assert(level_folder);
    return dynarray_data(level_folder->titles);
}

size_t level_folder_count(const LevelFolder *level_folder)
{
    trace_assert(level_folder);
    return dynarray_count(level_folder->filenames);
}
