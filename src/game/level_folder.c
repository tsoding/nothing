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

void level_folder_read(const char *dirpath, LevelFolder *folder)
{
    DIR *level_dir = opendir(dirpath);
    if (level_dir == NULL) {
        log_fail("Can't open asset folder: %s\n", dirpath);
        abort();
    }

    dynarray_clear(&folder->filepaths);

    char filepath[METADATA_FILEPATH_MAX_SIZE];
    for (struct dirent *d = readdir(level_dir);
         d != NULL;
         d = readdir(level_dir)) {
        if (*d->d_name == '.') continue;

        snprintf(filepath, METADATA_FILEPATH_MAX_SIZE,
                 "%s/%s", dirpath, d->d_name);
        dynarray_push(&folder->filepaths, filepath);
    }

    closedir(level_dir);
}
