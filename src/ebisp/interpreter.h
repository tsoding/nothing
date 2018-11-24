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
struct EvalResult
wrong_argument_type(Gc *gc, const char *type, struct Expr obj);
struct EvalResult
wrong_number_of_arguments(Gc *gc, long int count);
struct EvalResult
not_implemented(Gc *gc);

struct EvalResult
car(void *param, Gc *gc, struct Scope *scope, struct Expr args);

struct EvalResult eval(Gc *gc, struct Scope *scope, struct Expr expr);

void load_std_library(Gc *gc, struct Scope *scope);

struct EvalResult
unpack_args(struct Gc *gc, const char *format, struct Expr args, ...);

#endif  // INTERPRETER_H_
