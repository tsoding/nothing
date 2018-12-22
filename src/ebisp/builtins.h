#ifndef BUILTINS_H_
#define BUILTINS_H_

#include "expr.h"

bool equal(struct Expr obj1, struct Expr obj2);

bool nil_p(struct Expr obj);
bool symbol_p(struct Expr obj);
bool string_p(struct Expr obj);
bool number_p(struct Expr obj);
bool cons_p(struct Expr obj);
bool list_p(struct Expr obj);
bool list_of_symbols_p(struct Expr obj);
bool lambda_p(struct Expr obj);

bool is_lambda(struct Cons *cons);
bool is_special(const char *name);

long int length_of_list(struct Expr obj);

struct Expr assoc(struct Expr key, struct Expr alist);

// TODO(#568): list() function is obsolete because of format_list()
struct Expr list(Gc *gc, size_t n, ...);

#endif  // BUILTINS_H_
