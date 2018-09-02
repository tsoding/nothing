#ifndef INTERPRETER_H_
#define INTERPRETER_H_

#include <stdbool.h>
#include "expr.h"

struct EvalResult
{
    bool is_error;
    struct Expr expr;
    const char *error;
};

struct EvalResult eval_success(struct Expr expr);
struct EvalResult eval_failure(const char *error, struct Expr expr);

struct EvalResult eval(struct Expr scope, struct Expr expr);

#endif  // INTERPRETER_H_
