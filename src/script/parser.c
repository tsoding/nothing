#include <assert.h>
#include <stdlib.h>
#include <ctype.h>

#include "script/parser.h"

static void skip_whitespaces(const char *str, size_t *cursor, size_t n)
{
    assert(str);
    assert(cursor);

    while (*cursor < n && isspace(str[*cursor])) {
        (*cursor)++;
    }
}

struct ParseResult create_expr_from_str(const char *str,
                                        size_t *cursor,
                                        size_t n)
{
    assert(str);
    assert(cursor);

    /* TODO: create_expr_from_str doesn't no support comments */

    skip_whitespaces(str, cursor, n);
    if (*cursor >= n) {
        return parse_failure("EOF", *cursor);
    }

    switch (str[*cursor]) {
    case '(': {
        (*cursor)++;
        struct ParseResult car = create_expr_from_str(str, cursor, n);
        if (car.is_error) {
            return car;
        }

        skip_whitespaces(str, cursor, n);
        if (*cursor >= n) {
            return parse_failure("EOF", *cursor);
        }

        if (str[*cursor] != '.') {
            return parse_failure("Expected .", *cursor);
        }
        (*cursor)++;

        skip_whitespaces(str, cursor, n);
        if (*cursor >= n) {
            return parse_failure("EOF", *cursor);
        }

        struct ParseResult cdr = create_expr_from_str(str, cursor, n);
        if (cdr.is_error) {
            return cdr;
        }

        skip_whitespaces(str, cursor, n);
        if (*cursor >= n) {
            return parse_failure("EOF", *cursor);
        }

        if (str[*cursor] != ')') {
            return parse_failure("Expected )", *cursor);
        }

        (*cursor)++;

        return parse_success(cons_as_expr(create_cons(car.expr, cdr.expr)));
    }

    case '"': {
        /* TODO: parser does not support escaped string characters */
        const size_t str_begin = *cursor + 1;
        size_t str_end = str_begin;

        while(str_end < n && str[str_end] != '"') {
            str_end++;
        }

        if (str_end >= n) {
            return parse_failure("Unclosed string", str_begin);
        }

        *cursor = str_end + 1;

        return parse_success(
            atom_as_expr(
                create_string_atom(str + str_begin, str + str_end)));
    }

    default: {
        if (isdigit(str[*cursor])) {
            const char *nptr = str + *cursor;
            char *endptr = 0;
            const float x = strtof(nptr, &endptr);

            if (nptr == endptr) {
                return parse_failure("Number expected", *cursor);
            }

            *cursor += (size_t) (endptr - nptr);

            return parse_success(atom_as_expr(create_number_atom(x)));
        } else if (isalpha(str[*cursor])) {
            /* TODO(#289): create_expr_from_str does not support symbols */
            return parse_failure("Symbols are not supported", *cursor);
        }
    }
    }

    return parse_failure("Unexpected sequence of characters", *cursor);
}

struct ParseResult parse_success(struct Expr expr)
{
    struct ParseResult result = {
        .is_error = false,
        .expr = expr
    };

    return result;
}

struct ParseResult parse_failure(const char *error,
                                 size_t error_cursor)
{
    struct ParseResult result = {
        .is_error = true,
        {
            .error = error,
            .error_cursor = error_cursor
        }
    };

    return result;
}

void print_parse_error(FILE *stream,
                       const char *str,
                       struct ParseResult result)
{
    if (!result.is_error) {
        return;
    }

    /* TODO: print_parse_error doesn't support colors */
    /* TODO: print_parse_error doesn't support multiple lines */

    fprintf(stream, "%s\n", str);
    for (size_t i = 0; i < result.error_cursor; ++i) {
        fprintf(stream, " ");
    }
    fprintf(stream, "^\n");
    fprintf(stream, "%s\n", result.error);
}
