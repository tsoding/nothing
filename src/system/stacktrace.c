#include <stdio.h>
#ifdef __GNUC__
#include <execinfo.h>
#endif
#include <unistd.h>

#include "./stacktrace.h"

#define N 10

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
