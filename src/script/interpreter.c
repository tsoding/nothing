#include <assert.h>
#include <math.h>
#include <string.h>

#include "./expr.h"
#include "./interpreter.h"

struct EvalResult eval_success(struct Expr expr)
{
    struct EvalResult result = {
        .is_error = false,
        .expr = expr,
        .error = NULL
    };

    return result;
}

struct EvalResult eval_failure(const char *error, struct Expr expr)
{
    struct EvalResult result = {
        .is_error = true,
        .error = error,
        .expr = expr
    };

    return result;
}

static struct EvalResult eval_atom(struct Expr scope, struct Atom *atom)
{
    /* TODO: eval_atom is not implemented */
    assert(atom);
    (void) scope;
    return eval_failure("not implemented", void_expr());
}

static struct EvalResult eval_args(struct Expr scope, struct Expr args)
{
    (void) scope;
    (void) args;

    switch(args.type) {
    case EXPR_ATOM:
        return eval_atom(scope, args.atom);

    case EXPR_CONS: {
        struct EvalResult car = eval(scope, args.cons->car);
        if (car.is_error) {
            return car;
        }

        struct EvalResult cdr = eval_args(scope, args.cons->cdr);
        if (cdr.is_error) {
            return cdr;
        }

        /* TODO: memory leak */
        return eval_success(cons_as_expr(create_cons(car.expr, cdr.expr)));
    }

    default: {}
    }

    return eval_failure("Unexpected expression", args);
}

static struct EvalResult plus_op(struct Expr scope, struct Expr args)
{
    (void) scope;
    (void) args;
    /* TODO: plus_op is not implemented*/
    return eval_failure("not implemnted", void_expr());
}

static struct EvalResult eval_funcall(struct Expr scope, struct Cons *cons)
{
    assert(cons);
    (void) scope;

    if (cons->car.type != EXPR_ATOM && cons->car.atom->type != ATOM_SYMBOL) {
        return eval_failure("Not a function", cons->car);
    }

    if (strcmp(cons->car.atom->sym, "+")) {
        struct EvalResult args = eval_args(scope, cons->cdr);
        if (args.is_error) {
            return args;
        }
        return plus_op(scope, args.expr);
    }

    return eval_failure("Unknown function", cons->car);
}

struct EvalResult eval(struct Expr scope, struct Expr expr)
{
    switch(expr.type) {
    case EXPR_ATOM:
        return eval_atom(scope, expr.atom);

    case EXPR_CONS:
        return eval_funcall(scope, expr.cons);

    default: {}
    }

    return eval_failure("Unexpected expression", expr);
}
