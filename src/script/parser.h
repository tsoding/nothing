#ifndef PARSER_H_
#define PARSER_H_

#include <stdio.h>
#include <stdbool.h>
#include "script/expr.h"

struct ParseResult
{
    bool is_error;
    union {
        struct Expr expr;
        struct {
            const char *error;
            size_t error_cursor;
        };
    };
};

struct ParseResult parse_success(struct Expr expr);
struct ParseResult parse_failure(const char *error,
                                 size_t error_cursor);

struct ParseResult create_expr_from_str(const char *str,
                                        size_t *cursor,
                                        size_t n);

void print_parse_error(FILE *stream, const char *str, struct ParseResult result);

#endif  // PARSER_H_
