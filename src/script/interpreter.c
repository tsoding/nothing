#include <assert.h>
#include <math.h>
#include <string.h>

#include "./builtins.h"
#include "./expr.h"
#include "./interpreter.h"
#include "./scope.h"

struct EvalResult eval_success(struct Expr expr, struct Expr scope)
{
    struct EvalResult result = {
        .is_error = false,
        .expr = expr,
        .scope = scope,
    };

    return result;
}

struct EvalResult eval_failure(struct Expr error, struct Expr scope)
{
    struct EvalResult result = {
        .is_error = true,
        .expr = error,
        .scope = scope
    };

    return result;
}

static struct EvalResult length(Gc *gc, struct Expr scope, struct Expr obj)
{
    if (!list_p(obj)) {
        return eval_failure(list(gc, 3,
                                 SYMBOL(gc, "wrong-argument-type"),
                                 SYMBOL(gc, "listp"),
                                 obj),
                            scope);
    }

    long int count = 0;

    while (!nil_p(obj)) {
        count++;
        obj = obj.cons->cdr;
    }

    return eval_success(NUMBER(gc, count), scope);
}

static struct EvalResult eval_atom(Gc *gc, struct Expr scope, struct Atom *atom)
{
    (void) scope;
    (void) gc;

    switch (atom->type) {
    case ATOM_NUMBER:
    case ATOM_STRING:
        return eval_success(atom_as_expr(atom), scope);

    case ATOM_SYMBOL: {
        if (nil_p(atom_as_expr(atom))) {
            return eval_success(atom_as_expr(atom), scope);
        }

        struct Expr value = get_scope_value(scope, atom_as_expr(atom));

        if (nil_p(value)) {
            return eval_failure(CONS(gc,
                                     SYMBOL(gc, "void-variable"),
                                     atom_as_expr(atom)),
                                scope);
        }

        return eval_success(value.cons->cdr, scope);
    }
    }

    return eval_failure(CONS(gc,
                             SYMBOL(gc, "unexpected-expression"),
                             atom_as_expr(atom)),
                        scope);
}

static struct EvalResult eval_all_args(Gc *gc, struct Expr scope, struct Expr args)
{
    (void) scope;
    (void) args;

    switch(args.type) {
    case EXPR_ATOM:
        return eval_atom(gc, scope, args.atom);

    case EXPR_CONS: {
        struct EvalResult car = eval(gc, scope, args.cons->car);
        if (car.is_error) {
            return car;
        }

        struct EvalResult cdr = eval_all_args(gc, scope, args.cons->cdr);
        if (cdr.is_error) {
            return cdr;
        }

        return eval_success(cons_as_expr(create_cons(gc, car.expr, cdr.expr)), scope);
    }

    default: {}
    }

    return eval_failure(CONS(gc,
                             SYMBOL(gc, "unexpected-expression"),
                             args),
                        scope);
}

static struct EvalResult plus_op(Gc *gc, struct Expr args, struct Expr scope)
{
    long int result = 0.0f;

    while (!nil_p(args)) {
        if (args.type != EXPR_CONS) {
            return eval_failure(CONS(gc,
                                     SYMBOL(gc, "expected-cons"),
                                     args),
                                scope);
        }

        if (args.cons->car.type != EXPR_ATOM ||
            args.cons->car.atom->type != ATOM_NUMBER) {
            return eval_failure(CONS(gc,
                                     SYMBOL(gc, "expected-number"),
                                     args.cons->car),
                                scope);
        }

        result += args.cons->car.atom->num;
        args = args.cons->cdr;
    }

    return eval_success(atom_as_expr(create_number_atom(gc, result)), scope);
}

static struct EvalResult eval_funcall(Gc *gc, struct Expr scope, struct Cons *cons)
{
    assert(cons);
    (void) scope;

    if (!symbol_p(cons->car)) {
        return eval_failure(CONS(gc,
                                 SYMBOL(gc, "expected-symbol"),
                                 cons->car),
                            scope);
    }

    /* TODO(#323): set builtin function is not implemented */
    if (strcmp(cons->car.atom->sym, "+") == 0) {
        struct EvalResult args = eval_all_args(gc, scope, cons->cdr);
        if (args.is_error) {
            return args;
        }
        return plus_op(gc, args.expr, scope);
    } else if (strcmp(cons->car.atom->sym, "set") == 0) {
        struct Expr args = cons->cdr;
        struct EvalResult n = length(gc, scope, args);

        if (n.is_error) {
            return n;
        }
        scope = n.scope;

        if (n.expr.atom->num != 2) {
            return eval_failure(list(gc, 3,
                                     SYMBOL(gc, "wrong-number-of-arguments"),
                                     SYMBOL(gc, "set"),
                                     NUMBER(gc, n.expr.atom->num)),
                                scope);
        }

        struct Expr name = args.cons->car;
        if (!symbol_p(name)) {
            return eval_failure(list(gc, 3,
                                     SYMBOL(gc, "wrong-type-argument"),
                                     SYMBOL(gc, "symbolp"),
                                     name),
                                scope);
        }

        struct EvalResult value = eval(gc, scope, args.cons->cdr.cons->car);
        if (value.is_error) {
            return value;
        }
        scope = value.scope;

        return eval_success(value.expr, set_scope_value(gc, scope, name, value.expr));
    }

    return eval_failure(CONS(gc,
                             SYMBOL(gc, "unknown-function"),
                             cons->car),
                        scope);
}

struct EvalResult eval(Gc *gc, struct Expr scope, struct Expr expr)
{
    switch(expr.type) {
    case EXPR_ATOM:
        return eval_atom(gc, scope, expr.atom);

    case EXPR_CONS:
        return eval_funcall(gc, scope, expr.cons);

    default: {}
    }

    return eval_failure(CONS(gc,
                             SYMBOL(gc, "unexpected-expression"),
                             expr),
                        scope);
}
