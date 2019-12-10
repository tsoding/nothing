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

    dynarray_clear(&folder->metadatas);

    char filepath[METADATA_FILEPATH_MAX_SIZE];
    LevelMetadata metadata;
    for (struct dirent *d = readdir(level_dir);
         d != NULL;
         d = readdir(level_dir)) {
        if (*d->d_name == '.') continue;

        snprintf(filepath, METADATA_FILEPATH_MAX_SIZE,
                 "%s/%s", dirpath, d->d_name);

        if (metadata_load_from_file(&metadata, filepath) < 0) {
            log_warn("Can't read level: %s\n", filepath);
            continue;
        }

        if(strcmp(metadata.version, VERSION) != 0) {
            log_warn(
                "Unsupported version for level [%s]: Expected `%s`, got `%s`\n",
                filepath, VERSION, metadata.version);
            continue;
        }

        dynarray_push(&folder->metadatas, &metadata);
    }

    closedir(level_dir);
}
