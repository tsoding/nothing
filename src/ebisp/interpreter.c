#include <assert.h>
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
        list(gc, 3,
             SYMBOL(gc, "wrong-argument-type"),
             SYMBOL(gc, type),
             obj));
}

struct EvalResult
wrong_number_of_arguments(Gc *gc, long int count)
{
    return eval_failure(
        CONS(gc,
             SYMBOL(gc, "wrong-number-of-arguments"),
             NUMBER(gc, count)));
}

struct EvalResult
not_implemented(Gc *gc)
{
    return eval_failure(SYMBOL(gc, "not-implemented"));
}

static struct EvalResult length(Gc *gc, struct Expr obj)
{
    if (!list_p(obj)) {
        return wrong_argument_type(gc, "listp", obj);
    }

    return eval_success(NUMBER(gc, length_of_list(obj)));
}

static struct EvalResult eval_atom(Gc *gc, struct Scope *scope, struct Atom *atom)
{
    (void) scope;
    (void) gc;

    switch (atom->type) {
    case ATOM_NUMBER:
    case ATOM_STRING:
    case ATOM_NATIVE:
        return eval_success(atom_as_expr(atom));

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

static struct EvalResult
assoc_op(void *param, Gc *gc, struct Scope *scope, struct Expr args)
{
    (void) param;
    assert(gc);
    assert(scope);

    struct Expr key = NIL(gc);
    struct Expr alist = NIL(gc);
    struct EvalResult result = match_list(gc, "ee", args, &key, &alist);
    if (result.is_error) {
        return result;
    }

    return eval_success(assoc(key, alist));
}

static struct EvalResult
plus_op(void *param, Gc *gc, struct Scope *scope, struct Expr args)
{
    (void) param;
    assert(gc);
    assert(scope);

    long int result = 0L;

    while (!nil_p(args)) {
        if (!cons_p(args)) {
            return wrong_argument_type(gc, "consp", args);
        }

        if (!number_p(CAR(args))) {
            return wrong_argument_type(gc, "numberp", CAR(args));
        }

        result += CAR(args).atom->num;
        args = CDR(args);
    }

    return eval_success(NUMBER(gc, result));
}

static struct EvalResult call_lambda(Gc *gc,
                                     struct Scope *scope,
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

    struct Expr vars = lambda.cons->cdr.cons->car;

    if (length_of_list(args) != length_of_list(vars)) {
        return eval_failure(CONS(gc,
                                 SYMBOL(gc, "wrong-number-of-arguments"),
                                 NUMBER(gc, length_of_list(args))));
    }

    push_scope_frame(gc, scope, vars, args);
    struct Expr body = lambda.cons->cdr.cons->cdr;

    struct EvalResult result = eval_success(NIL(gc));

    while (!nil_p(body)) {
        result = eval(gc, scope, body.cons->car);
        if (result.is_error) {
            return result;
        }
        body = body.cons->cdr;
    }

    pop_scope_frame(gc, scope);

    return result;
}

static struct EvalResult call_callable(Gc *gc,
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

    return call_lambda(gc, scope, callable_result.expr, args_result.expr);
}

static struct Expr
lambda(Gc *gc, struct Expr args, struct Expr body)
{
    return CONS(gc,
                SYMBOL(gc, "lambda"),
                CONS(gc, args, body));
}

static struct EvalResult eval_block(Gc *gc, struct Scope *scope, struct Expr block)
{
    assert(gc);
    assert(scope);

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

static struct EvalResult
foo(void *param, Gc *gc, struct Scope *scope, struct Expr args)
{
    (void) param;
    assert(gc);
    assert(scope);

    return eval_success(args);
}

static struct EvalResult eval_funcall(Gc *gc, struct Scope *scope, struct Cons *cons)
{
    assert(cons);
    (void) scope;

    if (symbol_p(cons->car)) {
        /* TODO: eval_funcall contains some special forms that are not native function of stdlib */
        if (strcmp(cons->car.atom->sym, "set") == 0) {
            struct Expr args = cons->cdr;
            struct EvalResult n = length(gc, args);

            if (n.is_error) {
                return n;
            }

            if (n.expr.atom->num != 2) {
                return eval_failure(list(gc, 3,
                                         SYMBOL(gc, "wrong-number-of-arguments"),
                                         SYMBOL(gc, "set"),
                                         NUMBER(gc, n.expr.atom->num)));
            }

            struct Expr name = args.cons->car;
            if (!symbol_p(name)) {
                return eval_failure(list(gc, 3,
                                         SYMBOL(gc, "wrong-type-argument"),
                                         SYMBOL(gc, "symbolp"),
                                         name));
            }

            struct EvalResult value = eval(gc, scope, args.cons->cdr.cons->car);
            if (value.is_error) {
                return value;
            }

            set_scope_value(gc, scope, name, value.expr);

            return eval_success(value.expr);
        } else if (strcmp(cons->car.atom->sym, "quote") == 0) {
            /* TODO(#334): quote does not check the amout of it's arguments */
            return eval_success(cons->cdr.cons->car);
        } else if (strcmp(cons->car.atom->sym, "begin") == 0) {
            return eval_block(gc, scope, CDR(cons_as_expr(cons)));
        } else if (is_lambda(cons)) {
            /* TODO(#335): lambda special form doesn't check if it forms a callable object */
            return eval_success(cons_as_expr(cons));
        } else if (strcmp(cons->car.atom->sym, "defun") == 0) {
            struct Expr name = NIL(gc);
            struct Expr args = NIL(gc);
            struct Expr body = NIL(gc);

            /* TODO(#554): defun doesn't support functions with empty body because of #545 */
            struct EvalResult result = match_list(gc, "ee*", cons->cdr, &name, &args, &body);
            if (result.is_error) {
                return result;
            }

            return eval(gc, scope,
                        list(gc, 3,
                             SYMBOL(gc, "set"),
                             name,
                             lambda(gc, args, body)));
        } else if (strcmp(cons->car.atom->sym, "when") == 0) {
            struct Expr condition = NIL(gc);
            struct Expr body = NIL(gc);

            struct EvalResult result = match_list(
                gc, "e*", cons->cdr, &condition, &body);
            if (result.is_error) {
                return result;
            }

            result = eval(gc, scope, condition);
            if (result.is_error) {
                return result;
            }

            if (!nil_p(result.expr)) {
                return eval_block(gc, scope, body);
            }

            return eval_success(NIL(gc));
        }
    }

    return call_callable(gc, scope, cons->car, cons->cdr);
}

struct EvalResult eval(Gc *gc, struct Scope *scope, struct Expr expr)
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
                             expr));
}

struct EvalResult
car(void *param, Gc *gc, struct Scope *scope, struct Expr args)
{
    (void) param;
    assert(gc);
    assert(scope);

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

static struct EvalResult
list_op(void *param, Gc *gc, struct Scope *scope, struct Expr args)
{
    assert(gc);
    assert(scope);
    (void) param;

    return eval_success(args);
}

/* TODO(#536): greaterThan does not support arbitrary amount of arguments */
static struct EvalResult
greaterThan(void *param, Gc *gc, struct Scope *scope, struct Expr args)
{
    assert(gc);
    assert(scope);
    (void) param;

    long int x = 0, y = 0;

    struct EvalResult result = match_list(gc, "dd", args, &x, &y);
    if (result.is_error) {
        return result;
    }

    if (x > y) {
        return eval_success(T(gc));
    } else {
        return eval_success(NIL(gc));
    }
}

void load_std_library(Gc *gc, struct Scope *scope)
{
    set_scope_value(
        gc,
        scope,
        SYMBOL(gc, "car"),
        NATIVE(gc, car, NULL));
    set_scope_value(
        gc,
        scope,
        SYMBOL(gc, ">"),
        NATIVE(gc, greaterThan, NULL));
    set_scope_value(
        gc,
        scope,
        SYMBOL(gc, "+"),
        NATIVE(gc, plus_op, NULL));
    set_scope_value(
        gc,
        scope,
        SYMBOL(gc, "list"),
        NATIVE(gc, list_op, NULL));
    set_scope_value(
        gc,
        scope,
        SYMBOL(gc, "t"),
        SYMBOL(gc, "t"));
    set_scope_value(
        gc,
        scope,
        SYMBOL(gc, "nil"),
        SYMBOL(gc, "nil"));
    set_scope_value(
        gc,
        scope,
        SYMBOL(gc, "assoc"),
        NATIVE(gc, assoc_op, NULL));
    set_scope_value(
        gc,
        scope,
        SYMBOL(gc, "foo"),
        NATIVE(gc, foo, NULL));
}

struct EvalResult
match_list(struct Gc *gc, const char *format, struct Expr xs, ...)
{
    va_list args_list;
    va_start(args_list, xs);

    long int i = 0;
    for (i = 0; *format != 0 && !nil_p(xs); ++i) {
        if (!cons_p(xs)) {
            va_end(args_list);
            return wrong_argument_type(gc, "consp", xs);
        }

        struct Expr x = CAR(xs);

        switch (*format) {
        case 'd': {
            if (!number_p(x)) {
                va_end(args_list);
                return wrong_argument_type(gc, "numberp", x);
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
        return wrong_number_of_arguments(gc, i);
    }

    va_end(args_list);
    return eval_success(NIL(gc));
}

static struct Expr
format_list_rec(Gc *gc, const char *format, va_list args)
{
    assert(gc);
    assert(format);

    if (*format == 0) {
        return NIL(gc);
    }

    switch (*format) {
    case 'd': {
        long int p = va_arg(args, long int);
        return CONS(gc, NUMBER(gc, p),
                    format_list_rec(gc, format + 1, args));
    }

    case 's': {
        const char* p = va_arg(args, const char*);
        return CONS(gc, STRING(gc, p),
                    format_list_rec(gc, format + 1, args));
    }

    case 'q': {
        const char* p = va_arg(args, const char*);
        return CONS(gc, SYMBOL(gc, p),
                    format_list_rec(gc, format + 1, args));
    }

    case 'e': {
        struct Expr p = va_arg(args, struct Expr);
        return CONS(gc, p, format_list_rec(gc, format + 1, args));
    }

    default: {
        fprintf(stderr, "Wrong format parameter: %c\n", *format);
        assert(0);
    }
    }
}

struct Expr
format_list(Gc *gc, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    struct Expr result = format_list_rec(gc, format, args);
    va_end(args);

    return result;
}
