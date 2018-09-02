#ifndef SCOPE_H_
#define SCOPE_H_

#include "expr.h"
#include "builtins.h"

struct Expr empty_scope(void);
struct Expr get_scope_value(struct Expr scope, struct Expr name);
struct Expr set_scope_value(struct Expr scope, struct Expr name, struct Expr value);
struct Expr push_scope_frame(struct Expr scope);
struct Expr pop_scope_frame(struct Expr scope);


#endif  // SCOPE_H_
