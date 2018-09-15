#ifndef SCOPE_H_
#define SCOPE_H_

#include "expr.h"
#include "builtins.h"

struct Scope
{
    struct Expr expr;
};

// Scope is a stack of alists
// (((y . 20))
//  ((x . 10)
//   (name . "Alexey")))

struct Expr get_scope_value(const struct Scope *scope, struct Expr name);
void set_scope_value(Gc *gc, struct Scope *scope, struct Expr name, struct Expr value);


#endif  // SCOPE_H_
