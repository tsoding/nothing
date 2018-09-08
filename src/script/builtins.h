#ifndef BUILTINS_H_
#define BUILTINS_H_

#include "expr.h"

bool equal(struct Expr obj1, struct Expr obj2);

bool nil_p(struct Expr obj);
bool symbol_p(struct Expr obj);
bool cons_p(struct Expr obj);

/// You should never detroy the result of assoc because it always
/// returns the subexpression of alist
struct Expr assoc(struct Expr key, struct Expr alist);

#endif  // BUILTINS_H_
