#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "file.h"
#include "system/nth_alloc.h"
#include "system/stacktrace.h"
#include "lt_adapters.h"

#ifdef _WIN32

struct DIR
{
    HANDLE hFind;
    WIN32_FIND_DATA data;
    struct dirent *dirent;
};

DIR *opendir(const char *dirpath)
{
    trace_assert(dirpath);

    char buffer[MAX_PATH];
    snprintf(buffer, MAX_PATH, "%s\\*", dirpath);

    DIR *dir = nth_calloc(1, sizeof(DIR));

    dir->hFind = FindFirstFile(buffer, &dir->data);
    if (dir->hFind == INVALID_HANDLE_VALUE) {
        goto fail;
    }

    return dir;

fail:
    if (dir) {
        free(dir);
    }

    return NULL;
}

struct dirent *readdir(DIR *dirp)
{
    trace_assert(dirp);

    if (dirp->dirent == NULL) {
        dirp->dirent = nth_calloc(1, sizeof(struct dirent));
    } else {
        if(!FindNextFile(dirp->hFind, &dirp->data)) {
            return NULL;
        }
    }

    memset(dirp->dirent->d_name, 0, sizeof(dirp->dirent->d_name));

    strncpy(
        dirp->dirent->d_name,
        dirp->data.cFileName,
        sizeof(dirp->dirent->d_name) - 1);

    return dirp->dirent;
}

void closedir(DIR *dirp)
{
    trace_assert(dirp);

    FindClose(dirp->hFind);
    if (dirp->dirent) {
        free(dirp->dirent);
    }
    free(dirp);
}

#endif

String read_whole_file(Memory *memory, const char *filepath)
{
    trace_assert(filepath);

    String result = string(0, NULL);
    FILE *f = fopen(filepath, "rb");
    if (!f) goto end;
    if (fseek(f, 0, SEEK_END) < 0) goto end;
    long m = ftell(f);
    if (m < 0) goto end;
    if (fseek(f, 0, SEEK_SET) < 0) goto end;
    result.count = (size_t) m;
    char *buffer = memory_alloc(memory, result.count);
    size_t n = fread(buffer, 1, result.count, f);
    trace_assert(n == result.count);
    result.data = buffer;

end:
    if (f) fclose(f);
    return result;
}
