#ifndef SCOPE_H_
#define SCOPE_H_

#include "expr.h"
#include "builtins.h"

// Scope is a stack of alists
// (((y . 20))
//  ((x . 10)
//   (name . "Alexey")))

struct Expr get_scope_value(struct Expr scope, struct Expr name);
struct Expr set_scope_value(struct Expr scope, struct Expr name, struct Expr value);


#endif  // SCOPE_H_
