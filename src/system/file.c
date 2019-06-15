#include <stdio.h>
#include <stdlib.h>
#ifdef __linux__
#include <sys/stat.h>
#include <sys/types.h>
#elif defined(_WIN32)
#include <Windows.h>
#define WINDOWS_TICK 10000000
#define SEC_TO_UNIX_EPOCH 11644473600LL
#endif // __linux__

#include "system/stacktrace.h"
#include "file.h"

time_t last_modified(const char *filepath)
{
    trace_assert(filepath);

#ifdef __linux__
    struct stat attr;
    stat(filepath, &attr);
    return attr.st_mtime;
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
    if (!hFile) {
        return 0;
    }
    FILETIME filetime = { 0 };
    if (!GetFileTime(hFile, NULL, NULL, &filetime)) {
        CloseHandle(hFile);
        return 0;
    }
    CloseHandle(hFile);
    unsigned long long mod_time = filetime.dwHighDateTime;
    mod_time <<= 32;
    mod_time |= filetime.dwLowDateTime;
    // Taken from https://stackoverflow.com/a/6161842/1901561
    return mod_time / WINDOWS_TICK - SEC_TO_UNIX_EPOCH;
#endif // __linux__
}
