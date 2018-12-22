#include <assert.h>

#include "ebisp/gc.h"
#include "ebisp/interpreter.h"
#include "ebisp/builtins.h"
#include "ebisp/scope.h"

#include "std.h"

static struct Expr
lambda(Gc *gc, struct Expr args, struct Expr body)
{
    return CONS(gc,
                SYMBOL(gc, "lambda"),
                CONS(gc, args, body));
}

static struct EvalResult
quasiquote(void *param, Gc *gc, struct Scope *scope, struct Expr expr)
{
    (void) param;
    assert(gc);
    assert(scope);
    (void) expr;

    /* TODO(#582): quasiquote special form is not implemented */

    return not_implemented(gc);
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

static struct EvalResult
list_op(void *param, Gc *gc, struct Scope *scope, struct Expr args)
{
    assert(gc);
    assert(scope);
    (void) param;

    return eval_success(args);
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
set(void *param, Gc *gc, struct Scope *scope, struct Expr args)
{
    (void) param;
    assert(gc);
    assert(scope);

    const char *name = NULL;
    struct Expr value = void_expr();
    struct EvalResult result = match_list(gc, "qe", args, &name, &value);
    if (result.is_error) {
        return result;
    }

    result = eval(gc, scope, value);
    if (result.is_error) {
        return result;
    }

    set_scope_value(gc, scope, SYMBOL(gc, name), result.expr);

    return eval_success(result.expr);
}

static struct EvalResult
quote(void *param, Gc *gc, struct Scope *scope, struct Expr args)
{
    (void) param;
    assert(gc);
    assert(scope);

    struct Expr expr = void_expr();
    struct EvalResult result = match_list(gc, "e", args, &expr);
    if (result.is_error) {
        return result;
    }

    return eval_success(expr);
}

static struct EvalResult
begin(void *param, Gc *gc, struct Scope *scope, struct Expr args)
{
    (void) param;
    assert(gc);
    assert(scope);

    struct Expr block = void_expr();
    struct EvalResult result = match_list(gc, "*", args, &block);
    if (result.is_error) {
        return result;
    }

    return eval_block(gc, scope, block);
}

static struct EvalResult
defun(void *param, Gc *gc, struct Scope *scope, struct Expr args)
{
    (void) param;
    assert(gc);
    assert(scope);

    struct Expr name = void_expr();
    struct Expr args_list = void_expr();
    struct Expr body = void_expr();

    struct EvalResult result = match_list(gc, "ee*", args, &name, &args_list, &body);
    if (result.is_error) {
        return result;
    }

    return eval(gc, scope,
                list(gc, 3,
                     SYMBOL(gc, "set"),
                     name,
                     lambda(gc, args_list, body)));
}

void load_std_library(Gc *gc, struct Scope *scope)
{
    set_scope_value(gc, scope, SYMBOL(gc, "car"), NATIVE(gc, car, NULL));
    set_scope_value(gc, scope, SYMBOL(gc, ">"), NATIVE(gc, greaterThan, NULL));
    set_scope_value(gc, scope, SYMBOL(gc, "+"), NATIVE(gc, plus_op, NULL));
    set_scope_value(gc, scope, SYMBOL(gc, "list"), NATIVE(gc, list_op, NULL));
    set_scope_value(gc, scope, SYMBOL(gc, "t"), SYMBOL(gc, "t"));
    set_scope_value(gc, scope, SYMBOL(gc, "nil"), SYMBOL(gc, "nil"));
    set_scope_value(gc, scope, SYMBOL(gc, "assoc"), NATIVE(gc, assoc_op, NULL));
    set_scope_value(gc, scope, SYMBOL(gc, "quasiquote"), NATIVE(gc, quasiquote, NULL));
    set_scope_value(gc, scope, SYMBOL(gc, "set"), NATIVE(gc, set, NULL));
    set_scope_value(gc, scope, SYMBOL(gc, "quote"), NATIVE(gc, quote, NULL));
    set_scope_value(gc, scope, SYMBOL(gc, "begin"), NATIVE(gc, begin, NULL));
    set_scope_value(gc, scope, SYMBOL(gc, "defun"), NATIVE(gc, defun, NULL));
}
