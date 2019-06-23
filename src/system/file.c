#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "system/nth_alloc.h"
#ifdef __linux__
#include <sys/stat.h>
#include <sys/types.h>
#elif defined(_WIN32)
#include <Windows.h>
#define WINDOWS_TICK 10000000
#define SEC_TO_UNIX_EPOCH 11644473600LL
#endif

#include "system/stacktrace.h"
#include "file.h"

int last_modified(const char *filepath, time_t *time)
{
    trace_assert(filepath);
    trace_assert(time);

#ifdef __linux__

    struct stat attr;
    if (stat(filepath, &attr) < 0) {
        // errno is set by stat
        return -1;
    }
    *time = attr.st_mtime;
    return 0;

#elif defined(_WIN32)

    // CreateFile opens file (see flag OPEN_EXISTING)
    HANDLE hFile = CreateFile(
        filepath,
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );
    if (hFile == INVALID_HANDLE_VALUE) {
        // TODO(#900): convert GetLastError() to errno
        // for now let's just assume that file was not found.
        errno = ENOENT;
        return -1;
    }
    FILETIME filetime = { 0 };
    BOOL res = GetFileTime(hFile, NULL, NULL, &filetime);
    CloseHandle(hFile);
    if (!res) {
        errno = EPERM;
        return -1;
    }
    unsigned long long mod_time = filetime.dwHighDateTime;
    mod_time <<= 32;
    mod_time |= filetime.dwLowDateTime;
    // Taken from https://stackoverflow.com/a/6161842/1901561
    *time = mod_time / WINDOWS_TICK - SEC_TO_UNIX_EPOCH;
    return 0;

#elif defined(__APPLE__)

    // TODO(#901): implement last_modified for Mac OS X
    #warning last_modified is not implemented
    return -1;

#else

    #error Unsupported OS
    return -1;

#endif
}

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
