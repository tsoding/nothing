#include "./scope.h"

struct Expr empty_scope(void)
{
    return CONS(NIL, NIL);
}

struct Expr get_scope_value(struct Expr scope, struct Expr name)
{
    switch (scope.type) {
    case EXPR_CONS: {
        struct Expr value = assoc(name, scope.cons->car);
        return nil_p(value) ? get_scope_value(scope.cons->cdr, name) : value;
    } break;

    default:
        return scope;
    }
}

struct Expr set_scope_value(struct Expr scope, struct Expr name, struct Expr value)
{
    (void) name;
    (void) value;

    /* TODO: set_scope_value is not implemented */

    return scope;
}

struct Expr push_scope_frame(struct Expr scope)
{
    return CONS(empty_scope(), scope);
}

struct Expr pop_scope_frame(struct Expr scope)
{
    if (scope.type == EXPR_CONS) {
        return scope.cons->cdr;
    } else {
        return scope;
    }
}
