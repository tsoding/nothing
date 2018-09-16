#ifndef INTERPRETER_H_
#define INTERPRETER_H_

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "expr.h"

struct Scope;
struct Gc;


struct EvalResult eval_success(struct Expr expr);
struct EvalResult eval_failure(struct Expr expr);

struct EvalResult eval(Gc *gc, struct Scope *scope, struct Expr expr);

#endif  // INTERPRETER_H_
