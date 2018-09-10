#include <assert.h>
#include <math.h>
#include <string.h>

#include "./builtins.h"
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

static struct EvalResult eval_atom(Gc *gc, struct Expr scope, struct Atom *atom)
{
    (void) scope;

    /* TODO(#314): Evaluating symbols is not implemented */
    switch (atom->type) {
    case ATOM_NUMBER:
    case ATOM_SYMBOL:
    case ATOM_STRING:
        /* TODO: get rid of unnecessary clone_expr from interpreter */
        return eval_success(clone_expr(gc, atom_as_expr(atom)));
    }

    return eval_failure("Unexpected expression", clone_expr(gc, atom_as_expr(atom)));
}

static struct EvalResult eval_args(Gc *gc, struct Expr scope, struct Expr args)
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

        struct EvalResult cdr = eval_args(gc, scope, args.cons->cdr);
        if (cdr.is_error) {
            return cdr;
        }

        return eval_success(cons_as_expr(create_cons(gc, car.expr, cdr.expr)));
    }

    default: {}
    }

    return eval_failure("Unexpected expression", clone_expr(gc, args));
}

static struct EvalResult plus_op(Gc *gc, struct Expr args)
{
    float result = 0.0f;

    while (!nil_p(args)) {
        if (args.type != EXPR_CONS) {
            return eval_failure("Expected cons", clone_expr(gc, args));
        }

        if (args.cons->car.type != EXPR_ATOM ||
            args.cons->car.atom->type != ATOM_NUMBER) {
            return eval_failure("Expected number", clone_expr(gc, args.cons->car));
        }

        result += args.cons->car.atom->num;
        args = args.cons->cdr;
    }

    return eval_success(atom_as_expr(create_number_atom(gc, result)));
}

static struct EvalResult eval_funcall(Gc *gc, struct Expr scope, struct Cons *cons)
{
    assert(cons);
    (void) scope;

    if (!symbol_p(cons->car)) {
        return eval_failure("Expected symbol", clone_expr(gc, cons->car));
    }

    /* TODO: set builtin function is not implemented */
    /* depends on #317 */
    if (strcmp(cons->car.atom->sym, "+") == 0) {
        struct EvalResult args = eval_args(gc, scope, cons->cdr);
        if (args.is_error) {
            return args;
        }
        return plus_op(gc, args.expr);
    }

    return eval_failure("Unknown function", clone_expr(gc, cons->car));
}

/* TODO(#317): eval does not return new scope after the evaluation */
struct EvalResult eval(Gc *gc, struct Expr scope, struct Expr expr)
{
    switch(expr.type) {
    case EXPR_ATOM:
        return eval_atom(gc, scope, expr.atom);

    case EXPR_CONS:
        return eval_funcall(gc, scope, expr.cons);

    default: {}
    }

    return eval_failure("Unexpected expression", clone_expr(gc, expr));
}

void print_eval_error(FILE *stream, struct EvalResult result)
{
    if (!result.is_error) {
        return;
    }

    fprintf(stream, "%s\n", result.error);
    print_expr_as_sexpr(result.expr);
}
