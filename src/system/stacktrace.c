#include <stdlib.h>
#include <stdio.h>
#ifdef __GNUC__
#include <execinfo.h>
#endif
#ifdef WINDOWS_OS
    #include <io.h>
#else
    #include <unistd.h>
#endif

#include "./stacktrace.h"

#define N 100

void print_stacktrace(void)
{
#ifdef __GNUC__
    void *array[N];
    int size;

    size = backtrace(array, N);

    if (size <= 0) {
        return;
    }

    fprintf(stderr, "Stacktrace: \n");
    backtrace_symbols_fd(array + 1, size - 1, STDERR_FILENO);
#endif
}

void __trace_assert(const char *file, int line, const char *function, const char *message)
{
    fprintf(
        stderr,
        "%s:%d: %s: Assertion `%s' failed\n",
        file, line,
        function,
        message);
    print_stacktrace();
    exit(1);
}
