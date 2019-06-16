#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
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
        // TODO: convert GetLastError() to errno
        // for now let's just assume that file was not found.
        errno = ENOENT;
        return 0;
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

    // TODO: implement last_modified for Mac OS X
    #warning last_modified is not implemented
    return -1;

#else

    #error Unsupported OS
    return -1;

#endif
}
