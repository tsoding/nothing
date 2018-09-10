#include "./scope.h"

struct Expr get_scope_value(struct Expr scope, struct Expr name)
{
    if (cons_p(scope)) {
        struct Expr value = assoc(name, scope.cons->car);
        return nil_p(value) ? get_scope_value(scope.cons->cdr, name) : value;
    }

    return scope;
}

struct Expr set_scope_value(Gc *gc, struct Expr scope, struct Expr name, struct Expr value)
{
    if (cons_p(scope)) {
        if (!nil_p(assoc(name, scope.cons->car)) || nil_p(scope.cons->cdr)) {
            return CONS(gc,
                        CONS(gc, CONS(gc, name, value), scope.cons->car),
                        scope.cons->cdr);
        } else {
            return CONS(gc,
                        scope.cons->car,
                        set_scope_value(gc, scope.cons->cdr, name, value));
        }
    } else {
        return CONS(gc,
                    CONS(gc, CONS(gc, name, value), NIL(gc)),
                    scope);
    }
}
