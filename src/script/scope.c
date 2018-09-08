#include "./scope.h"

static struct Expr find_frame_with_name(struct Expr scope, struct Expr name)
{
    (void) name;

    /* TODO: find_frame_with_name is not implemented */
    return scope;
}

struct Expr empty_scope(void)
{
    return CONS(NIL, NIL);      /* '(()) */
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

void set_scope_value(struct Expr scope, struct Expr name, struct Expr value)
{
    struct Expr frame = find_frame_with_name(scope, name);
    push(CONS(name, value), frame);
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
