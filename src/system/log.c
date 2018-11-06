#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "./log.h"

#define SEVERITY_FAIL "FAIL"
#define SEVERITY_WARN "WARN"
#define SEVERITY_INFO "INFO"

static int log_core(const char *severity, const char *format, va_list args)
{
    int err = fprintf(stderr, "[%s] ", severity);
    if (err < 0) {
        return err;
    }

    return vfprintf(stderr, format, args);
}

int log_fail(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int err = log_core(SEVERITY_FAIL, format, args);
    va_end(args);
    return err;
}

int log_warn(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int err = log_core(SEVERITY_WARN, format, args);
    va_end(args);
    return err;
}

int log_info(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int err = log_core(SEVERITY_INFO, format, args);
    va_end(args);
    return err;
}
