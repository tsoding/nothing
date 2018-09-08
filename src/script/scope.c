#include "./scope.h"

struct Expr get_scope_value(struct Expr scope, struct Expr name)
{
    if (cons_p(scope)) {
        struct Expr value = assoc(name, scope.cons->car);
        return nil_p(value) ? get_scope_value(scope.cons->cdr, name) : value;
    }

    return scope;
}

struct Expr set_scope_value(struct Expr scope, struct Expr name, struct Expr value)
{
    if (cons_p(scope)) {
        if (!nil_p(assoc(name, scope.cons->car))) {
            return CONS(CONS(name, value), scope);
        } else {
            scope.cons->cdr = set_scope_value(scope.cons->cdr, name, value);
            return scope;
        }
    } else {
        /* TODO(#318): set_scope_value creates redundant global scopes */
        return CONS(CONS(name, value), NIL);
    }
}
