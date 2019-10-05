#include <stdio.h>
#include <stdbool.h>
#include "system/stacktrace.h"
#include <ctype.h>
#include <stdlib.h>

#include "./tokenizer.h"

static struct Token token(const char *begin, const char *end)
{
    struct Token token = {
        .begin = begin,
        .end = end
    };

    return token;
}

static bool is_symbol_char(char x)
{
    static const char forbidden_symbol_chars[] = {
        '(', ')', '"', '\'', ';', '`', ','
    };
    static const size_t n = sizeof(forbidden_symbol_chars) / sizeof(char);

    for (size_t i = 0; i < n; ++i) {
        if (x == forbidden_symbol_chars[i] || isspace(x)) {
            return false;
        }
    }

    return true;
}

static const char *skip_whitespace(const char *str)
{
    trace_assert(str);

    while(*str != 0 && isspace(*str)) {
        str++;
    }

    return str;
}

static const char *next_quote(const char *str)
{
    trace_assert(str);

    while(*str != 0 && *str != '"') {
        str++;
    }

    return str;
}

static const char *skip_until_newline(const char *str)
{
    trace_assert(str);

    while(*str != 0 && *str != '\n') {
        str++;
    }

    return str;
}

static const char *next_non_symbol(const char *str)
{
    trace_assert(str);

    while(*str != 0 && is_symbol_char(*str)) {
        str++;
    }

    return str;
}

struct Token next_token(const char *str)
{
    trace_assert(str);

    str = skip_whitespace(str);
    if (*str == 0) {
        return token(str, str);
    }

    while (*str != 0 && *str == ';') {
        str = skip_until_newline(str + 1);
        str = skip_whitespace(str);
    }

    switch (*str) {
    case '(':
    case ')':
    case '.':
    case '\'':
    case '`':
    case ',':
        return token(str, str + 1);

    case '"': {
        const char *str_end = next_quote(str + 1);
        return token(str, *str_end == 0 ? str_end : str_end + 1);
    }

    default:
        return token(str, next_non_symbol(str + 1));
    }
}
