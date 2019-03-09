#include <stdio.h>
#include <stdlib.h>

#include "system/stacktrace.h"
#include "system/lt.h"
#include "system/nth_alloc.h"
#include "system/line_stream.h"
#include "system/str.h"
#include "dynarray.h"

#include "./level_folder.h"

#define LEVEL_FOLDER_MAX_LENGTH 512

struct LevelFolder
{
    Lt *lt;
    Dynarray *items;
};

LevelFolder *create_level_folder(const char *dirpath)
{
    trace_assert(dirpath);

    Lt *lt = create_lt();
    if (lt == NULL) {
        return NULL;
    }

    LevelFolder *level_folder = PUSH_LT(
        lt,
        nth_alloc(sizeof(LevelFolder)),
        free);
    if (level_folder == NULL) {
        RETURN_LT(lt, NULL);
    }
    level_folder->lt = lt;

    level_folder->items = PUSH_LT(
        lt,
        create_dynarray(sizeof(const char*)),
        destroy_dynarray);
    if (level_folder->items == NULL) {
        RETURN_LT(lt, NULL);
    }

    char path[LEVEL_FOLDER_MAX_LENGTH];
    snprintf(path, LEVEL_FOLDER_MAX_LENGTH, "%s/meta.txt", dirpath);

    LineStream *meta = PUSH_LT(
        lt,
        create_line_stream(
            path,
            "r",
            LEVEL_FOLDER_MAX_LENGTH),
        destroy_line_stream);
    if (meta == NULL) {
        RETURN_LT(lt, NULL);
    }

    const char *line = line_stream_next(meta);
    while (line) {
        snprintf(path, LEVEL_FOLDER_MAX_LENGTH, "%s/%s", dirpath, line);
        line = PUSH_LT(lt, string_duplicate(trim_endline(path), NULL), free);
        if (line == NULL) {
            RETURN_LT(lt, NULL);
        }

        dynarray_push(level_folder->items, &line);

        line = line_stream_next(meta);
    }

    destroy_line_stream(RELEASE_LT(lt, meta));

    return level_folder;
}

void destroy_level_folder(LevelFolder *level_folder)
{
    trace_assert(level_folder);
    RETURN_LT0(level_folder->lt);
}

const char **level_folder_files(const LevelFolder *level_folder)
{
    trace_assert(level_folder);
    return dynarray_data(level_folder->items);
}

size_t level_folder_count(const LevelFolder *level_folder)
{
    trace_assert(level_folder);
    return dynarray_count(level_folder->items);
}
