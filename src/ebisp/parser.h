#ifndef PARSER_H_
#define PARSER_H_

#include <stdio.h>
#include <stdbool.h>
#include "ebisp/expr.h"
#include "ebisp/tokenizer.h"

struct ParseResult
{
    bool is_error;
    const char *end;
    union {
        struct Expr expr;
        const char *error_message;
    };
};

struct ParseResult parse_success(struct Expr expr,
                                 const char *end);
struct ParseResult parse_failure(const char *error,
                                 const char *end);


struct ParseResult read_expr_from_string(Gc *gc, const char *str);
struct ParseResult read_all_exprs_from_string(Gc *gc, const char *str);

struct ParseResult read_expr_from_file(Gc *gc, const char *filename);
struct ParseResult read_all_exprs_from_file(Gc *gc, const char *filename);

void print_parse_error(FILE *stream,
                       const char *str,
                       struct ParseResult result);
// TODO(#668): there is no way to print parse error after parsing a file

#endif  // PARSER_H_
