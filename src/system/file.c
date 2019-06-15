#include <stdio.h>
#include <stdlib.h>
#ifdef __linux__
#include <sys/stat.h>
#include <sys/types.h>
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
#endif // __linux__
}
