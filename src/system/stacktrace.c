#include <stdlib.h>
#include <stdio.h>
#if defined(__GNUC__) && defined(__linux__)
#include <execinfo.h>
#include <unistd.h>
#define N 100
#endif

#include "./stacktrace.h"


void print_stacktrace(void)
{
#if defined(__GNUC__) && defined(__linux__)
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
    abort();
}
