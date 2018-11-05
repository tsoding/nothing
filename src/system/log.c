#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "./log.h"

/* TODO: all instances of fprintf(stderr, ...) should be replaced with log_error (except ebisp) */

int log_error(const char *format, ...)
{
    va_list args;
    va_start(args, format);

    int err = fprintf(stderr, "[ERROR] ");
    if (err < 0) {
        return err;
    }

    return vfprintf(stderr, format, args);
}
