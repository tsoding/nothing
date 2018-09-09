#ifndef INTERPRETER_H_
#define INTERPRETER_H_

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "expr.h"

struct EvalResult
{
    bool is_error;
    struct Expr expr;
    const char *error;
};

struct EvalResult eval_success(struct Expr expr);
struct EvalResult eval_failure(const char *error, struct Expr expr);

// expr returned by eval should always be destroyed with destroy_expr
struct EvalResult eval(Gc *gc, struct Expr scope, struct Expr expr);

void print_eval_error(FILE *stream, struct EvalResult result);

#endif  // INTERPRETER_H_
