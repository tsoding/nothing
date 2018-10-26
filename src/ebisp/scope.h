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

struct Scope create_scope(Gc *gc);

struct Expr get_scope_value(const struct Scope *scope, struct Expr name);
void set_scope_value(Gc *gc, struct Scope *scope, struct Expr name, struct Expr value);
void push_scope_frame(Gc *gc, struct Scope *scope, struct Expr vars, struct Expr args);
void pop_scope_frame(Gc *gc, struct Scope *scope);

#endif  // SCOPE_H_
