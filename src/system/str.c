#include "system/stacktrace.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "system/stacktrace.h"

#include "str.h"
#include "system/nth_alloc.h"

char *string_duplicate(const char *str,
                       const char *str_end)
{
    trace_assert(str);

    if (str_end != NULL && str > str_end) {
        return NULL;
    }

    const size_t n = str_end == NULL ? strlen(str) : (size_t) (str_end - str);
    char *dup_str = nth_calloc(1, sizeof(char) * (n + 1));
    if (dup_str == NULL) {
        return NULL;
    }

    memcpy(dup_str, str, n);
    dup_str[n] = '\0';

    return dup_str;
}

char *trim_endline(char *s)
{
    const size_t n = strlen(s);

    if (n == 0) {
        return s;
    }

    if (s[n - 1] == '\n') {
        s[n - 1] = '\0';
    }

    return s;
}
