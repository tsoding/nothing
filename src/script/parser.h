#ifndef PARSER_H_
#define PARSER_H_

#include <stdbool.h>
#include "script/expr.h"

struct ParseResult
{
    bool is_error;
    union {
        struct Expr expr;
        const char *error;
    };
};

struct ParseResult parse_success(struct Expr expr);
struct ParseResult parse_failure(const char *error);

struct ParseResult create_expr_from_str(const char *str,
                                        size_t *cursor,
                                        size_t n);


#endif  // PARSER_H_
