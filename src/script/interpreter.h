#ifndef INTERPRETER_H_
#define INTERPRETER_H_

#include "expr.h"

bool equal(struct Expr obj1, struct Expr obj2);

struct Expr assoc(struct Expr key);

struct Expr eval(struct Expr expr);

#endif  // INTERPRETER_H_
