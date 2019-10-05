#include "system/stacktrace.h"
#include <math.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>

#include "./builtins.h"
#include "./expr.h"
#include "./interpreter.h"
#include "./scope.h"

struct EvalResult eval_success(struct Expr expr)
{
    struct EvalResult result = {
        .is_error = false,
        .expr = expr,
    };

    return result;
}

struct EvalResult eval_failure(struct Expr error)
{
    struct EvalResult result = {
        .is_error = true,
        .expr = error,
    };

    return result;
}

struct EvalResult
wrong_argument_type(Gc *gc, const char *type, struct Expr obj)
{
    return eval_failure(
        list(gc, "qqe", "wrong-argument-type", type, obj));
}

struct EvalResult
wrong_integer_of_arguments(Gc *gc, long int count)
{
    return eval_failure(
        CONS(gc,
             SYMBOL(gc, "wrong-integer-of-arguments"),
             INTEGER(gc, count)));
}

struct EvalResult
not_implemented(Gc *gc)
{
    return eval_failure(SYMBOL(gc, "not-implemented"));
}

struct EvalResult
read_error(Gc *gc, const char *error_message, long int character)
{
    return eval_failure(
        list(gc, "qsd", "read-error", error_message, character));
}

static struct EvalResult eval_atom(Gc *gc, struct Scope *scope, struct Atom *atom)
{
    (void) scope;
    (void) gc;

    switch (atom->type) {
    case ATOM_INTEGER:
    case ATOM_REAL:
    case ATOM_STRING:
    case ATOM_LAMBDA:
    case ATOM_NATIVE: {
        return eval_success(atom_as_expr(atom));
    }

    case ATOM_SYMBOL: {
        struct Expr value = get_scope_value(scope, atom_as_expr(atom));

        if (nil_p(value)) {
            return eval_failure(CONS(gc,
                                     SYMBOL(gc, "void-variable"),
                                     atom_as_expr(atom)));
        }

        return eval_success(value.cons->cdr);
    }
    }

    return eval_failure(CONS(gc,
                             SYMBOL(gc, "unexpected-expression"),
                             atom_as_expr(atom)));
}

static struct EvalResult eval_all_args(Gc *gc, struct Scope *scope, struct Expr args)
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

        return eval_success(cons_as_expr(create_cons(gc, car.expr, cdr.expr)));
    }

    default: {}
    }

    return eval_failure(CONS(gc,
                             SYMBOL(gc, "unexpected-expression"),
                             args));
}

static struct EvalResult call_lambda(Gc *gc,
                                     struct Expr lambda,
                                     struct Expr args) {
    if (!lambda_p(lambda)) {
        return eval_failure(CONS(gc,
                                 SYMBOL(gc, "expected-callable"),
                                 lambda));
    }

    if (!list_p(args)) {
        return eval_failure(CONS(gc,
                                 SYMBOL(gc, "expected-list"),
                                 args));
    }

    struct Expr vars = lambda.atom->lambda.args_list;

    if (length_of_list(args) != length_of_list(vars)) {
        return eval_failure(CONS(gc,
                                 SYMBOL(gc, "wrong-integer-of-arguments"),
                                 INTEGER(gc, length_of_list(args))));
    }

    struct Scope scope = {
        .expr = lambda.atom->lambda.envir
    };
    push_scope_frame(gc, &scope, vars, args);

    struct Expr body = lambda.atom->lambda.body;

    struct EvalResult result = eval_success(NIL(gc));

    while (!nil_p(body)) {
        result = eval(gc, &scope, body.cons->car);
        if (result.is_error) {
            return result;
        }
        body = body.cons->cdr;
    }

    return result;
}

static struct EvalResult eval_funcall(Gc *gc,
                                      struct Scope *scope,
                                      struct Expr callable_expr,
                                      struct Expr args_expr) {
    struct EvalResult callable_result = eval(gc, scope, callable_expr);
    if (callable_result.is_error) {
        return callable_result;
    }

    struct EvalResult args_result = symbol_p(callable_expr) && is_special(callable_expr.atom->sym)
        ? eval_success(args_expr)
        : eval_all_args(gc, scope, args_expr);

    if (args_result.is_error) {
        return args_result;
    }

    if (callable_result.expr.type == EXPR_ATOM &&
        callable_result.expr.atom->type == ATOM_NATIVE) {
        return ((NativeFunction)callable_result.expr.atom->native.fun)(
            callable_result.expr.atom->native.param, gc, scope, args_result.expr);
    }

    return call_lambda(gc, callable_result.expr, args_result.expr);
}


struct EvalResult eval_block(Gc *gc, struct Scope *scope, struct Expr block)
{
    trace_assert(gc);
    trace_assert(scope);

    if (!list_p(block)) {
        return wrong_argument_type(gc, "listp", block);
    }

    struct Expr head = block;
    struct EvalResult eval_result = eval_success(NIL(gc));

    while (cons_p(head)) {
        eval_result = eval(gc, scope, CAR(head));
        if (eval_result.is_error) {
            return eval_result;
        }

        head = CDR(head);
    }

    return eval_result;
}

struct EvalResult eval(Gc *gc, struct Scope *scope, struct Expr expr)
{
    switch(expr.type) {
    case EXPR_ATOM:
        return eval_atom(gc, scope, expr.atom);

    case EXPR_CONS:
        return eval_funcall(gc, scope, expr.cons->car, expr.cons->cdr);

    default: {}
    }

    return eval_failure(CONS(gc,
                             SYMBOL(gc, "unexpected-expression"),
                             expr));
}

struct EvalResult
car(void *param, Gc *gc, struct Scope *scope, struct Expr args)
{
    (void) param;
    trace_assert(gc);
    trace_assert(scope);

    struct Expr xs = NIL(gc);

    struct EvalResult result = match_list(gc, "e", args, &xs);
    if (result.is_error) {
        return result;
    }

    if (nil_p(xs)) {
        return eval_success(xs);
    }

    if (!cons_p(xs)) {
        return wrong_argument_type(gc, "consp", xs);
    }

    return eval_success(CAR(xs));
}

struct EvalResult
match_list(struct Gc *gc, const char *format, struct Expr xs, ...)
{
    va_list args_list;
    va_start(args_list, xs);

    // TODO(#1098): match_list does not support real numbers

    long int i = 0;
    for (i = 0; *format != 0 && !nil_p(xs); ++i) {
        if (!cons_p(xs)) {
            va_end(args_list);
            return wrong_argument_type(gc, "consp", xs);
        }

        struct Expr x = CAR(xs);

        switch (*format) {
        case 'd': {
            if (!integer_p(x)) {
                va_end(args_list);
                return wrong_argument_type(gc, "integerp", x);
            }

            long int *p = va_arg(args_list, long int *);
            if (p != NULL) {
                *p = x.atom->num;
            }
        } break;

        case 's': {
            if (!string_p(x)) {
                va_end(args_list);
                return wrong_argument_type(gc, "stringp", x);
            }

            const char **p = va_arg(args_list, const char**);
            if (p != NULL) {
                *p = x.atom->str;
            }
        } break;

        case 'q': {
            if (!symbol_p(x)) {
                va_end(args_list);
                return wrong_argument_type(gc, "symbolp", x);
            }

            const char **p = va_arg(args_list, const char**);
            if (p != NULL) {
                *p = x.atom->sym;
            }
        } break;

        case 'e': {
            struct Expr *p = va_arg(args_list, struct Expr*);
            *p = x;
        } break;

        case '*': {
            struct Expr *p = va_arg(args_list, struct Expr*);
            if (p != NULL) {
                *p = xs;
            }
            xs = NIL(gc);
        } break;
        }

        format++;
        if (!nil_p(xs)) {
            xs = CDR(xs);
        }
    }

    if (*format == '*' && nil_p(xs)) {
        struct Expr *p = va_arg(args_list, struct Expr*);
        if (p != NULL) {
            *p = NIL(gc);
        }
        format++;
    }

    if (*format != 0 || !nil_p(xs)) {
        va_end(args_list);
        return wrong_integer_of_arguments(gc, i);
    }

    va_end(args_list);
    return eval_success(NIL(gc));
}
