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
    struct Expr scope;
};

struct EvalResult eval_success(struct Expr expr, struct Expr scope);
struct EvalResult eval_failure(struct Expr expr, struct Expr scope);

struct EvalResult eval(Gc *gc, struct Expr scope, struct Expr expr);

#endif  // INTERPRETER_H_
