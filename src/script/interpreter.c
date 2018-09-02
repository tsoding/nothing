#include <assert.h>
#include <math.h>
#include <string.h>

#include "./interpreter.h"

struct EvalResult eval_success(struct Expr expr)
{
    struct EvalResult result = {
        .is_error = false,
        .expr = expr
    };

    return result;
}

struct EvalResult eval_failure(const char *error)
{
    struct EvalResult result = {
        .is_error = true,
        .error = error
    };

    return result;
}

static struct EvalResult eval_atom(struct Expr scope, struct Atom *atom)
{
    /* TODO: eval_atom is not implemented */
    assert(atom);
    (void) scope;
    return eval_failure("not implemented");
}

static struct EvalResult eval_funcall(struct Expr scope, struct Cons *cons)
{
    /* TODO: eval_funcall is not implemented */
    assert(cons);
    (void) scope;
    return eval_failure("not implemented");
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

    return eval_failure("eval: unexpected expression");
}
